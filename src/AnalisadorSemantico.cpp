// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#include "AnalisadorSemantico.hpp"
#include "Fase1_Lexico.hpp"
#include "LexerInterface.hpp"
#include "Gramatica.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include <functional>

// ============================================================
// FUNÇÃO 1: prepararEntradaSemantica
// Executa léxico → parser → valida (START)/(END)
// ============================================================
pair<vector<Token>, NoAST*> prepararEntradaSemantica(
    const string& arquivo,
    vector<string>& errosLex,
    vector<string>& errosSin)
{
    const string arquivoTokens = "tokens_temp_fase3.txt";

    cout << "\n[FASE 1] Iniciando analise lexica..." << endl;
    bool lexOk = executarFase1(arquivo, arquivoTokens);

    if (!lexOk) {
        errosLex.push_back("[ERRO LEXICO] Erros lexicos detectados. Analise sintatica abortada.");
    }

    cout << "\n[FASE 2] Iniciando analise sintatica..." << endl;

    // Constrói gramática e tabela LL(1)
    construirGramatica();
    calcularFirst();
    calcularFollow();
    construirTabelaLL1();

    // Lê os tokens
    vector<Token> tokens = lerTokens(arquivoTokens);

    if (tokens.empty()) {
        errosSin.push_back("[ERRO SINTATICO] Nenhum token foi gerado pelo lexico.");
        return {{}, nullptr};
    }

    // Valida (START) e (END) como primeira e última instrução
    bool temStart = false, temEnd = false;
    for (size_t i = 0; i + 2 < tokens.size(); i++) {
        if (tokens[i].tipo == "PARENTE_ESQ" &&
            tokens[i+1].tipo == "START" &&
            tokens[i+2].tipo == "PARENTE_DIR") {
            temStart = true;
            break;
        }
    }
    for (int i = (int)tokens.size() - 1; i >= 0; i--) {
        if (tokens[i].tipo == "END") {
            temEnd = true;
            break;
        }
    }
    if (!temStart)
        errosSin.push_back("[ERRO SINTATICO] Programa nao começa com '( START )'.");
    if (!temEnd)
        errosSin.push_back("[ERRO SINTATICO] Programa nao termina com '( END )'.");

    // Gera a AST base
    NoAST* arvore = gerarArvore(tokens, errosSin);

    return {tokens, arvore};
}

// ============================================================
// AUXILIAR: inferir tipo de um literal
// ============================================================
static string inferirTipoLiteral(const string& valor) {
    if (valor == "true" || valor == "false") return "bool";
    // real: contém ponto decimal
    for (char c : valor)
        if (c == '.') return "real";
    return "int";
}

// ============================================================
// AUXILIAR: regras de compatibilidade de tipos para operadores
// ============================================================
static string tipoResultadoOp(const string& op,
                               const string& tEsq,
                               const string& tDir,
                               int linha,
                               vector<string>& erros)
{
    // Operadores que exigem inteiros
    if (op == "/" || op == "%") {
        if (tEsq != "int" || tDir != "int") {
            erros.push_back(
                "[ERRO SEMANTICO] Linha " + to_string(linha) +
                ": Operador '" + op + "' requer operandos 'int', mas recebeu '" +
                tEsq + "' e '" + tDir + "'."
            );
            return "int"; // tenta continuar
        }
        return "int";
    }

    // Operadores relacionais → bool
    if (op == ">" || op == "<" || op == "=") {
        if ((tEsq == "bool") || (tDir == "bool")) {
            erros.push_back(
                "[ERRO SEMANTICO] Linha " + to_string(linha) +
                ": Operador relacional '" + op +
                "' nao pode ser usado com tipo 'bool'."
            );
        }
        return "bool";
    }

    // Divisão real |
    if (op == "|") {
        if (tEsq == "bool" || tDir == "bool") {
            erros.push_back(
                "[ERRO SEMANTICO] Linha " + to_string(linha) +
                ": Operador '|' (divisao real) nao pode ser usado com tipo 'bool'."
            );
            return "real";
        }
        return "real";
    }

    // +, -, *, ^
    if (op == "+" || op == "-" || op == "*" || op == "^") {
        if (tEsq == "bool" || tDir == "bool") {
            erros.push_back(
                "[ERRO SEMANTICO] Linha " + to_string(linha) +
                ": Operacao aritmetica '" + op +
                "' nao pode ser usada com tipo 'bool'."
            );
            return "int";
        }
        // int x int → int; qualquer real → real
        if (tEsq == "real" || tDir == "real") return "real";
        return "int";
    }

    return ""; // desconhecido
}

// ============================================================
// FUNÇÃO 2: construirTabelaSimbolos
// Percorre a AST e registra variáveis detectando erros
// ============================================================
TabelaSimbolos construirTabelaSimbolos(NoAST* arvore, vector<string>& errosSem) {
    TabelaSimbolos ts;
    if (!arvore) return ts;

    // Travessia recursiva
    function<void(NoAST*)> percorrer = [&](NoAST* no) {
        if (!no) return;

        // (V MEM) → definição de variável
        // Estrutura: KEY_MEM com filhos[0]=expressão, filhos[1]=VARIAVEL (destino)
        if (no->tipo == "KEY_MEM" && no->filhos.size() == 2) {
            NoAST* destino = no->filhos[1];
            if (destino && destino->tipo == "VARIAVEL") {
                // Tipo será preenchido pela verificação de tipos; por ora deixamos em aberto
                bool ok = ts.definir(destino->valor, "", no->linha);
                if (!ok) {
                    errosSem.push_back(
                        "[ERRO SEMANTICO] Linha " + to_string(no->linha) +
                        ": Variavel '" + destino->valor +
                        "' redefinida com tipo incompativel."
                    );
                }
            }
            // Percorre o filho de valor
            percorrer(no->filhos[0]);
        }
        // Uso de variável (fora de MEM destino)
        else if (no->tipo == "VARIAVEL") {
            if (!ts.estaInicializada(no->valor)) {
                errosSem.push_back(
                    "[ERRO SEMANTICO] Linha " + to_string(no->linha) +
                    ": Variavel '" + no->valor +
                    "' usada antes de ser definida com '(V MEM)'."
                );
            } else {
                ts.registrarUso(no->valor, no->linha);
            }
        }
        // Para KEY_RES valida o índice
        else if (no->tipo == "KEY_RES" && !no->filhos.empty()) {
            // Validação do índice feita no verificarTipos quando houver contexto de linha
            percorrer(no->filhos[0]);
        }
        else {
            // Para qualquer outro nó, percorre todos os filhos normalmente
            for (auto filho : no->filhos)
                percorrer(filho);
        }
    };

    // A raiz contém as instruções como filhos diretos
    for (auto filho : arvore->filhos)
        percorrer(filho);

    return ts;
}

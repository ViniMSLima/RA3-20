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

analisadorsemantico.cpp


// ============================================================
// FUNÇÃO 3: verificarTipos
// Inferência e validação de tipos — retorna mapa nó→tipo
// ============================================================
map<NoAST*, string> verificarTipos(
    NoAST* arvore,
    TabelaSimbolos& ts,
    vector<string>& errosSem)
{
    map<NoAST*, string> tipos;

    function<string(NoAST*)> inferir = [&](NoAST* no) -> string {
        if (!no) return "";

        // --- Literais ---
        if (no->tipo == "NUMERO" || no->tipo == "BOOLEANO") {
            string t = inferirTipoLiteral(no->valor);
            tipos[no] = t;
            return t;
        }

        // --- Variável ---
        if (no->tipo == "VARIAVEL") {
            string t = ts.getTipo(no->valor);
            tipos[no] = t.empty() ? "?" : t;
            return tipos[no];
        }

        // --- Operadores aritméticos / relacionais ---
        if (no->tipo == "OP_ARITMETICO" || no->tipo == "OP_RELACIONAL") {
            if (no->filhos.size() < 2) {
                tipos[no] = "?";
                return "?";
            }
            string tEsq = inferir(no->filhos[0]);
            string tDir = inferir(no->filhos[1]);
            string tRes = tipoResultadoOp(no->valor, tEsq, tDir, no->linha, errosSem);
            tipos[no] = tRes;
            return tRes;
        }

        // --- IF ---
        if (no->tipo == "KEY_IF") {
            if (no->filhos.size() < 2) { tipos[no] = "?"; return "?"; }
            string tCond = inferir(no->filhos[0]);
            string tAcao = inferir(no->filhos[1]);
            if (tCond != "bool") {
                errosSem.push_back(
                    "[ERRO SEMANTICO] Linha " + to_string(no->linha) +
                    ": Condicao do IF deve ser do tipo 'bool', mas e '" + tCond + "'."
                );
            }
            tipos[no] = tAcao;
            return tAcao;
        }

        // --- WHILE ---
        if (no->tipo == "KEY_WHILE") {
            if (no->filhos.size() < 2) { tipos[no] = "?"; return "?"; }
            string tCond = inferir(no->filhos[0]);
            string tAcao = inferir(no->filhos[1]);
            if (tCond != "bool") {
                errosSem.push_back(
                    "[ERRO SEMANTICO] Linha " + to_string(no->linha) +
                    ": Condicao do WHILE deve ser do tipo 'bool', mas e '" + tCond + "'."
                );
            }
            tipos[no] = tAcao;
            return tAcao;
        }

        // --- MEM: (V MEM) armazena e infere tipo da variável ---
        if (no->tipo == "KEY_MEM" && no->filhos.size() == 2) {
            string tValor = inferir(no->filhos[0]);
            NoAST* destino = no->filhos[1];

            if (destino && destino->tipo == "VARIAVEL") {
                string tipoAtual = ts.getTipo(destino->valor);
                if (!tipoAtual.empty() && tipoAtual != tValor) {
                    errosSem.push_back(
                        "[ERRO SEMANTICO] Linha " + to_string(no->linha) +
                        ": Variavel '" + destino->valor +
                        "' foi definida como '" + tipoAtual +
                        "' mas esta sendo atribuida com valor do tipo '" + tValor + "'."
                    );
                } else {
                    ts.setTipo(destino->valor, tValor);
                    tipos[destino] = tValor;
                }
            }

            tipos[no] = tValor;
            return tValor;
        }

        // --- RES ---
        if (no->tipo == "KEY_RES") {
            // Tipo de RES é desconhecido estaticamente (depende de qual linha retorna)
            // Assumimos "real" para compatibilidade (o buffer interno usa double)
            tipos[no] = "real";
            return "real";
        }

        // --- Raiz / outros ---
        for (auto filho : no->filhos) inferir(filho);
        tipos[no] = "";
        return "";
    };

    for (auto filho : arvore->filhos)
        inferir(filho);

    return tipos;
}

// ============================================================
// FUNÇÃO 4: gerarArvoreAtribuida
// Anota tipoSemantico em cada nó e salva o arquivo
// ============================================================
NoAST* gerarArvoreAtribuida(
    NoAST* arvore,
    TabelaSimbolos& ts,
    map<NoAST*, string>& tipos)
{
    (void)ts;    // ts disponível para extensões futuras
    (void)tipos; // usado abaixo via captura por referência no lambda

    // Anota tipoSemantico em cada nó que possui entrada no mapa
    function<void(NoAST*)> anotar = [&](NoAST* no) {
        if (!no) return;
        if (tipos.count(no)) no->tipoSemantico = tipos[no];
        for (auto filho : no->filhos) anotar(filho);
    };

    anotar(arvore);

    // Salva a árvore atribuída em Markdown
    ofstream out("arvore_atribuida.md");
    if (out.is_open()) {
        out << "# Árvore Sintática Atribuída\n\n";
        out << "Formato: `[tipo_sintatico:tipo_semantico] valor`\n\n";
        out << "```\n";
        arvore->imprimirArvoreAtribuida(out);
        out << "```\n";
        out.close();
        cout << "[ARTEFATO] Arvore atribuida salva em: arvore_atribuida.md" << endl;
    }

    return arvore;
}

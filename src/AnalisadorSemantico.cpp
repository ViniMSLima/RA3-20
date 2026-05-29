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

// ============================================================
// FUNÇÃO 5: gerarAssembly
// Gera código ARMv7 — SOMENTE quando sem erros
// ============================================================

// Estado interno do gerador
static int    s_labelCount  = 0;
static int    s_linhaAtual  = 0;
static ofstream s_saida;
static map<string, double> s_literais;
static map<string, int>    s_memorias;

static string criarLabel(const string& prefixo) {
    return prefixo + "_" + to_string(s_labelCount++);
}

static void gerarCodigo(NoAST* no);  // forward declaration

static void gerarCodigo(NoAST* no) {
    if (!no) return;

    // --- NÚMERO ---
    if (no->tipo == "NUMERO") {
        string lab = "lit_" + to_string(s_literais.size());
        s_literais[lab] = stod(no->valor);
        s_saida << "    LDR R0, =" << lab << "\n";
        s_saida << "    VLDR.64 D0, [R0]\n";
        s_saida << "    VPUSH {D0}\n";
    }
    // --- BOOLEANO ---
    else if (no->tipo == "BOOLEANO") {
        string lab = (no->valor == "true") ? "lit_ONE" : "lit_ZERO";
        s_saida << "    LDR R0, =" << lab << "\n";
        s_saida << "    VLDR.64 D0, [R0]\n";
        s_saida << "    VPUSH {D0}\n";
    }
    // --- VARIÁVEL (leitura) ---
    else if (no->tipo == "VARIAVEL") {
        s_memorias[no->valor] = 1;
        s_saida << "    LDR R0, =var_" << no->valor << "\n";
        s_saida << "    VLDR.64 D0, [R0]\n";
        s_saida << "    VPUSH {D0}\n";
    }
    // --- OPERADORES ARITMÉTICOS ---
    else if (no->tipo == "OP_ARITMETICO") {
        gerarCodigo(no->filhos[0]);
        gerarCodigo(no->filhos[1]);
        s_saida << "    VPOP {D1} @ Segundo operando\n";
        s_saida << "    VPOP {D0} @ Primeiro operando\n";

        const string& op = no->valor;

        if (op == "+") {
            s_saida << "    VADD.F64 D0, D0, D1\n";
            s_saida << "    VPUSH {D0}\n";
        } else if (op == "-") {
            s_saida << "    VSUB.F64 D0, D0, D1\n";
            s_saida << "    VPUSH {D0}\n";
        } else if (op == "*") {
            s_saida << "    VMUL.F64 D0, D0, D1\n";
            s_saida << "    VPUSH {D0}\n";
        } else if (op == "|") {
            // Divisão real
            s_saida << "    VDIV.F64 D0, D0, D1\n";
            s_saida << "    VPUSH {D0}\n";
        } else if (op == "/") {
            // Divisão inteira
            s_saida << "    VCVT.S32.F64 S0, D0 @ D0 -> int\n";
            s_saida << "    VCVT.S32.F64 S2, D1 @ D1 -> int\n";
            s_saida << "    VMOV R0, S0\n";
            s_saida << "    VMOV R1, S2\n";
            s_saida << "    SDIV R0, R0, R1      @ divisao inteira\n";
            s_saida << "    VMOV S0, R0\n";
            s_saida << "    VCVT.F64.S32 D0, S0  @ resultado -> double\n";
            s_saida << "    VPUSH {D0}\n";
        } else if (op == "%") {
            // Resto inteiro: rem = dividendo - (dividendo/divisor)*divisor
            s_saida << "    VCVT.S32.F64 S0, D0  @ dividendo -> int\n";
            s_saida << "    VCVT.S32.F64 S2, D1  @ divisor -> int\n";
            s_saida << "    VMOV R0, S0\n";
            s_saida << "    VMOV R1, S2\n";
            s_saida << "    SDIV R2, R0, R1      @ quociente\n";
            s_saida << "    MUL  R3, R2, R1      @ quociente * divisor\n";
            s_saida << "    SUB  R0, R0, R3      @ resto\n";
            s_saida << "    VMOV S0, R0\n";
            s_saida << "    VCVT.F64.S32 D0, S0  @ resto -> double\n";
            s_saida << "    VPUSH {D0}\n";
        } else if (op == "^") {
            // Potenciação por loop
            string lLoop = criarLabel("L_POW_LOOP");
            string lEnd  = criarLabel("L_POW_END");
            s_saida << "    VCVT.S32.F64 S4, D1  @ expoente -> int\n";
            s_saida << "    VMOV R0, S4           @ R0 = contador\n";
            s_saida << "    LDR R1, =lit_ONE\n";
            s_saida << "    VLDR.64 D2, [R1]      @ D2 = acumulador (1.0)\n";
            s_saida << lLoop << ":\n";
            s_saida << "    CMP R0, #0\n";
            s_saida << "    BLE " << lEnd << "\n";
            s_saida << "    VMUL.F64 D2, D2, D0  @ acumulador *= base\n";
            s_saida << "    SUB R0, R0, #1\n";
            s_saida << "    B " << lLoop << "\n";
            s_saida << lEnd << ":\n";
            s_saida << "    VPUSH {D2}\n";
        }
    }
    // --- OPERADORES RELACIONAIS ---
    else if (no->tipo == "OP_RELACIONAL") {
        gerarCodigo(no->filhos[0]);
        gerarCodigo(no->filhos[1]);
        s_saida << "    VPOP {D1}\n";
        s_saida << "    VPOP {D0}\n";
        s_saida << "    VCMP.F64 D0, D1\n";
        s_saida << "    VMRS APSR_nzcv, FPSCR\n";

        string lTrue = criarLabel("L_REL_TRUE");
        string lFim  = criarLabel("L_REL_END");

        if      (no->valor == ">") s_saida << "    BGT " << lTrue << "\n";
        else if (no->valor == "<") s_saida << "    BLT " << lTrue << "\n";
        else if (no->valor == "=") s_saida << "    BEQ " << lTrue << "\n";

        s_saida << "    LDR R0, =lit_ZERO\n";
        s_saida << "    VLDR.64 D0, [R0]\n";
        s_saida << "    VPUSH {D0}           @ falso: 0.0\n";
        s_saida << "    B " << lFim << "\n";
        s_saida << lTrue << ":\n";
        s_saida << "    LDR R0, =lit_ONE\n";
        s_saida << "    VLDR.64 D0, [R0]\n";
        s_saida << "    VPUSH {D0}           @ verdadeiro: 1.0\n";
        s_saida << lFim << ":\n";
    }
    // --- IF ---
    else if (no->tipo == "KEY_IF") {
        string lFim = criarLabel("L_END_IF");

        gerarCodigo(no->filhos[0]);           // condição
        s_saida << "    VPOP {D1} @ Resultado da condicao\n";
        s_saida << "    LDR R0, =lit_ZERO\n";
        s_saida << "    VLDR.64 D0, [R0]\n";
        s_saida << "    VCMP.F64 D1, D0\n";
        s_saida << "    VMRS APSR_nzcv, FPSCR\n";
        s_saida << "    BEQ " << lFim << " @ Se falso, pula acao\n";

        gerarCodigo(no->filhos[1]);           // ação
        s_saida << "    VPOP {D0} @ Descarta retorno da acao\n";

        s_saida << lFim << ":\n";
        s_saida << "    LDR R0, =lit_ZERO\n";
        s_saida << "    VLDR.64 D0, [R0]\n";
        s_saida << "    VPUSH {D0} @ Valor de retorno do IF\n";
    }
    // --- WHILE ---
    else if (no->tipo == "KEY_WHILE") {
        string lStart = criarLabel("L_START_WHILE");
        string lEnd   = criarLabel("L_END_WHILE");

        s_saida << lStart << ":\n";
        gerarCodigo(no->filhos[0]);           // condição
        s_saida << "    VPOP {D1} @ Resultado da condicao\n";
        s_saida << "    LDR R0, =lit_ZERO\n";
        s_saida << "    VLDR.64 D0, [R0]\n";
        s_saida << "    VCMP.F64 D1, D0\n";
        s_saida << "    VMRS APSR_nzcv, FPSCR\n";
        s_saida << "    BEQ " << lEnd << " @ Se falso, sai do laco\n";

        gerarCodigo(no->filhos[1]);           // ação
        s_saida << "    VPOP {D0} @ Descarta retorno (evita acumulo na pilha)\n";
        s_saida << "    B " << lStart << " @ Reinicia o laco\n";
        s_saida << lEnd << ":\n";

        s_saida << "    LDR R0, =lit_ZERO\n";
        s_saida << "    VLDR.64 D0, [R0]\n";
        s_saida << "    VPUSH {D0} @ Valor de retorno do WHILE\n";
    }
    // --- MEM (armazenamento) ---
    else if (no->tipo == "KEY_MEM" && no->filhos.size() == 2) {
        string var = no->filhos[1]->valor;
        s_memorias[var] = 1;

        gerarCodigo(no->filhos[0]);           // computa o valor
        s_saida << "    VPOP {D0} @ Pega o valor computado\n";
        s_saida << "    LDR R0, =var_" << var << "\n";
        s_saida << "    VSTR.64 D0, [R0] @ Armazena em " << var << "\n";
        s_saida << "    VPUSH {D0} @ Devolve para balancear a pilha\n";
    }
    // --- RES ---
    else if (no->tipo == "KEY_RES" && !no->filhos.empty()) {
        int n    = stoi(no->filhos[0]->valor);
        int alvo = s_linhaAtual - n;

        if (alvo < 0) {
            s_saida << "    LDR R0, =lit_ZERO @ RES fora do intervalo\n";
            s_saida << "    VLDR.64 D0, [R0]\n";
        } else {
            s_saida << "    LDR R0, =buffer_resultados\n";
            s_saida << "    MOV R1, #" << (alvo * 8) << " @ Offset da linha " << alvo << "\n";
            s_saida << "    ADD R0, R0, R1\n";
            s_saida << "    VLDR.64 D0, [R0]\n";
        }
        s_saida << "    VPUSH {D0}\n";
    }
}

void gerarAssembly(NoAST* arvoreAtribuida, const string& nomeArquivo) {
    // Reseta estado
    s_labelCount = 0;
    s_linhaAtual = 0;
    s_literais.clear();
    s_memorias.clear();

    s_saida.open(nomeArquivo);
    if (!s_saida.is_open()) {
        cerr << "[ERRO] Nao foi possivel criar o arquivo Assembly: " << nomeArquivo << endl;
        return;
    }

    // Constantes padrão
    s_literais["lit_ZERO"] = 0.0;
    s_literais["lit_ONE"]  = 1.0;

    // Cabeçalho ARMv7
    s_saida << ".global _start\n";
    s_saida << "_start:\n";
    s_saida << "    MOV SP, #0x00800000 @ Inicializa a pilha no topo\n\n";

    // Gera código para cada instrução do programa
    for (auto filho : arvoreAtribuida->filhos) {
        s_saida << "    @ --- INSTRUCAO " << s_linhaAtual << " ---\n";
        gerarCodigo(filho);

        // Salva resultado desta instrução no buffer histórico (para RES)
        s_saida << "    VPOP {D0} @ Pega o resultado da instrucao\n";
        s_saida << "    LDR R0, =buffer_resultados\n";
        s_saida << "    MOV R1, #" << (s_linhaAtual * 8) << "\n";
        s_saida << "    ADD R0, R0, R1\n";
        s_saida << "    VSTR.64 D0, [R0] @ Salva no historico\n";
        s_saida << "    VPUSH {D0}\n\n";

        s_linhaAtual++;
    }

    s_saida << "fim:\n    B fim\n\n";

    // Seção de dados
    s_saida << ".data\n";
    s_saida << ".align 3\n";
    s_saida << "buffer_resultados: .space 800 @ Buffer para 100 instrucoes\n";

    for (const auto& [nome, _] : s_memorias)
        s_saida << "var_" << nome << ": .double 0.0\n";

    for (const auto& [label, valor] : s_literais)
        s_saida << label << ": .double " << valor << "\n";

    s_saida.close();
    cout << "[ASSEMBLY] Codigo ARMv7 gerado com sucesso: " << nomeArquivo << endl;
}

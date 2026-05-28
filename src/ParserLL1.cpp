// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#include "Gramatica.hpp"

// ============================================================
// GRAMÁTICA LL(1) da linguagem RPN
// Terminais: START END NUMERO VARIAVEL BOOLEANO OP_ARITMETICO
//            OP_RELACIONAL KEY_IF KEY_WHILE KEY_MEM KEY_RES
//            PARENTE_ESQ PARENTE_DIR EOF
// ============================================================

Gramatica gramatica;

void construirGramatica() {
    gramatica.clear();

    // programa -> PARENTE_ESQ START PARENTE_DIR proximo_bloco
    gramatica["programa"].push_back({"PARENTE_ESQ", "START", "PARENTE_DIR", "proximo_bloco"});

    // proximo_bloco -> PARENTE_ESQ corpo_instrucao | ε
    gramatica["proximo_bloco"].push_back({"PARENTE_ESQ", "corpo_instrucao"});
    gramatica["proximo_bloco"].push_back({"EPSILON"});

    // corpo_instrucao -> END PARENTE_DIR | expressao PARENTE_DIR proximo_bloco
    gramatica["corpo_instrucao"].push_back({"END", "PARENTE_DIR"});
    gramatica["corpo_instrucao"].push_back({"expressao", "PARENTE_DIR", "proximo_bloco"});

    // expressao -> lista_operandos operador_final
    gramatica["expressao"].push_back({"lista_operandos", "operador_final"});

    // lista_operandos -> operando lista_operandos | ε
    gramatica["lista_operandos"].push_back({"operando", "lista_operandos"});
    gramatica["lista_operandos"].push_back({"EPSILON"});

    // operando -> NUMERO | VARIAVEL | BOOLEANO | PARENTE_ESQ expressao PARENTE_DIR
    gramatica["operando"].push_back({"NUMERO"});
    gramatica["operando"].push_back({"VARIAVEL"});
    gramatica["operando"].push_back({"BOOLEANO"});
    gramatica["operando"].push_back({"PARENTE_ESQ", "expressao", "PARENTE_DIR"});

    // operador_final -> OP_ARITMETICO | OP_RELACIONAL | KEY_IF | KEY_WHILE | KEY_MEM | KEY_RES
    gramatica["operador_final"].push_back({"OP_ARITMETICO"});
    gramatica["operador_final"].push_back({"OP_RELACIONAL"});
    gramatica["operador_final"].push_back({"KEY_IF"});
    gramatica["operador_final"].push_back({"KEY_WHILE"});
    gramatica["operador_final"].push_back({"KEY_MEM"});
    gramatica["operador_final"].push_back({"KEY_RES"});
}

// Conjunto dos terminais da gramática
bool validarTerminal(string simbolo) {
    static const set<string> terminais = {
        "START", "END", "NUMERO", "VARIAVEL", "BOOLEANO",
        "OP_ARITMETICO", "OP_RELACIONAL",
        "KEY_IF", "KEY_WHILE", "KEY_MEM", "KEY_RES",
        "PARENTE_ESQ", "PARENTE_DIR", "EOF"
    };
    return terminais.count(simbolo) > 0;
}

// ============================================================
// FIRST / FOLLOW / TABELA LL(1) — iguais à Fase 2
// ============================================================

map<string, set<string>> conjuntosFirst;

set<string> buscarFirst(string simbolo) {
    set<string> resultado;

    if (validarTerminal(simbolo) || simbolo == "EPSILON") {
        resultado.insert(simbolo);
        return resultado;
    }

    if (!conjuntosFirst[simbolo].empty())
        return conjuntosFirst[simbolo];

    for (const auto& producao : gramatica[simbolo]) {
        bool producaoPodeSerVazia = true;

        for (const string& palavra : producao) {
            set<string> firstDaPalavra = buscarFirst(palavra);

            for (const string& f : firstDaPalavra)
                if (f != "EPSILON") resultado.insert(f);

            if (firstDaPalavra.find("EPSILON") == firstDaPalavra.end()) {
                producaoPodeSerVazia = false;
                break;
            }
        }

        if (producaoPodeSerVazia) resultado.insert("EPSILON");
    }

    conjuntosFirst[simbolo] = resultado;
    return resultado;
}

void calcularFirst() {
    for (const auto& par : gramatica)
        buscarFirst(par.first);
}

map<string, set<string>> conjuntosFollow;

void calcularFollow() {
    conjuntosFollow["programa"].insert("EOF");

    bool mudou = true;
    while (mudou) {
        mudou = false;

        for (const auto& par : gramatica) {
            string ladoEsquerdo = par.first;

            for (const auto& producao : par.second) {
                for (size_t i = 0; i < producao.size(); ++i) {
                    string simboloAtual = producao[i];

                    if (validarTerminal(simboloAtual) || simboloAtual == "EPSILON") continue;

                    size_t tamanhoOriginal = conjuntosFollow[simboloAtual].size();

                    bool epsilonNoFirstDoResto = true;
                    for (size_t j = i + 1; j < producao.size(); ++j) {
                        string beta = producao[j];
                        set<string> firstBeta = buscarFirst(beta);

                        for (const string& f : firstBeta)
                            if (f != "EPSILON") conjuntosFollow[simboloAtual].insert(f);

                        if (firstBeta.find("EPSILON") == firstBeta.end()) {
                            epsilonNoFirstDoResto = false;
                            break;
                        }
                    }

                    if (epsilonNoFirstDoResto) {
                        for (const string& f : conjuntosFollow[ladoEsquerdo])
                            conjuntosFollow[simboloAtual].insert(f);
                    }

                    if (conjuntosFollow[simboloAtual].size() > tamanhoOriginal)
                        mudou = true;
                }
            }
        }
    }
}

map<string, map<string, vector<string>>> tabelaLL1;

void construirTabelaLL1() {
    for (const auto& par : gramatica) {
        string naoTerminal = par.first;

        for (const auto& producao : par.second) {
            set<string> firstDaProducao;
            bool producaoPodeSerVazia = true;

            for (const string& palavra : producao) {
                set<string> firstPalavra = buscarFirst(palavra);
                for (const string& f : firstPalavra)
                    if (f != "EPSILON") firstDaProducao.insert(f);

                if (firstPalavra.find("EPSILON") == firstPalavra.end()) {
                    producaoPodeSerVazia = false;
                    break;
                }
            }
            if (producaoPodeSerVazia) firstDaProducao.insert("EPSILON");

            for (const string& terminal : firstDaProducao) {
                if (terminal != "EPSILON") {
                    if (!tabelaLL1[naoTerminal][terminal].empty()) {
                        cerr << "[CONFLITO LL(1)] [" << naoTerminal << "]["
                             << terminal << "]" << endl;
                    } else {
                        tabelaLL1[naoTerminal][terminal] = producao;
                    }
                }
            }

            if (producaoPodeSerVazia) {
                for (const string& terminalFollow : conjuntosFollow[naoTerminal]) {
                    if (!tabelaLL1[naoTerminal][terminalFollow].empty()) {
                        if (tabelaLL1[naoTerminal][terminalFollow] != producao)
                            cerr << "[CONFLITO LL(1)] [" << naoTerminal << "]["
                                 << terminalFollow << "]" << endl;
                    } else {
                        tabelaLL1[naoTerminal][terminalFollow] = producao;
                    }
                }
            }
        }
    }
}

// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#include "LexerInterface.hpp"
#include <fstream>
#include <sstream>

vector<Token> lerTokens(string nomeArquivo) {
    vector<Token> listaGeralTokens;
    ifstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        cerr << "[ERRO] LexerInterface nao conseguiu abrir o arquivo de tokens: "
             << nomeArquivo << endl;
        return listaGeralTokens;
    }

    string linha;
    while (getline(arquivo, linha)) {
        if (linha.empty()) continue;

        // Remove '\r' de fins de linha Windows
        if (!linha.empty() && linha.back() == '\r')
            linha.pop_back();

        // Formato: TIPO,VALOR,LINHA
        size_t posVirgula1 = linha.find(',');
        size_t posVirgula2 = linha.find(',', posVirgula1 + 1);

        if (posVirgula1 == string::npos || posVirgula2 == string::npos) continue;

        string tipoLexer  = linha.substr(0, posVirgula1);
        string valorLexer = linha.substr(posVirgula1 + 1, posVirgula2 - posVirgula1 - 1);
        string linhaStr   = linha.substr(posVirgula2 + 1);

        // Ignora tokens de erro (já foram reportados pelo Fase1)
        if (tipoLexer == "ERRO_LEXICO") continue;

        Token t;
        t.valor = valorLexer;
        t.linha = stoi(linhaStr);

        // ============================================================
        // ADAPTER: Traduz tipos da Fase 1 para os tipos internos da Fase 2/3
        // ============================================================

        if (tipoLexer == "PARENTE") {
            t.tipo = (valorLexer == "(") ? "PARENTE_ESQ" : "PARENTE_DIR";
        }
        else if (tipoLexer == "NUMERO") {
            t.tipo = "NUMERO";
        }
        else if (tipoLexer == "VARIAVEL") {
            // Redundância defensiva: keywords que o léxico possa ter classificado como VARIAVEL
            if      (valorLexer == "MEM")   t.tipo = "KEY_MEM";
            else if (valorLexer == "RES")   t.tipo = "KEY_RES";
            else if (valorLexer == "START") t.tipo = "START";
            else if (valorLexer == "END")   t.tipo = "END";
            else if (valorLexer == "IF")    t.tipo = "KEY_IF";
            else if (valorLexer == "WHILE") t.tipo = "KEY_WHILE";
            else                            t.tipo = "VARIAVEL";
        }
        else if (tipoLexer == "OPERADOR") {
            if (valorLexer == ">" || valorLexer == "<" || valorLexer == "=")
                t.tipo = "OP_RELACIONAL";
            else
                t.tipo = "OP_ARITMETICO";
        }
        else if (tipoLexer == "KEYWORD") {
            if      (valorLexer == "RES")   t.tipo = "KEY_RES";
            else if (valorLexer == "MEM")   t.tipo = "KEY_MEM";
            else if (valorLexer == "IF")    t.tipo = "KEY_IF";
            else if (valorLexer == "WHILE") t.tipo = "KEY_WHILE";
            else if (valorLexer == "START") t.tipo = "START";
            else if (valorLexer == "END")   t.tipo = "END";
            else if (valorLexer == "true" || valorLexer == "false") t.tipo = "BOOLEANO";
            else                            t.tipo = tipoLexer;
        }
        else {
            t.tipo = tipoLexer; // Fallback
        }

        listaGeralTokens.push_back(t);
    }

    arquivo.close();
    cout << "[FASE 1 -> 2] " << listaGeralTokens.size()
         << " tokens traduzidos para analise sintatica." << endl;
    return listaGeralTokens;
}

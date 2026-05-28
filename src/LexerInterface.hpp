// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#ifndef LEXER_INTERFACE_HPP
#define LEXER_INTERFACE_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

// Estrutura padrão do Token (compatível com Fase 2)
struct Token {
    string tipo;   // Ex: NUMERO, PARENTE_ESQ, KEY_IF, OP_RELACIONAL, VARIAVEL
    string valor;  // Ex: 10, (, IF, >
    int    linha;  // Linha de origem no código-fonte
};

// Lê o arquivo de tokens gerado pela Fase 1 e traduz para o formato interno da Fase 2.
// Tokens do tipo ERRO_LEXICO são ignorados (já reportados pelo léxico).
vector<Token> lerTokens(string nomeArquivo);

#endif

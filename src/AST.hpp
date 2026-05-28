// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include "LexerInterface.hpp"
using namespace std;

// Nó da Árvore Sintática Abstrata / Árvore Sintática Atribuída
class NoAST {
public:
    string tipo;            // Tipo sintático: NUMERO, VARIAVEL, OP_ARITMETICO, KEY_IF...
    string valor;           // Valor lexical: "42", "X", "+", "IF"...
    string tipoSemantico;   // [FASE 3] Tipo inferido: "int", "real", "bool", "" = não inferido
    int    linha;           // [FASE 3] Linha de origem para mensagens de erro
    vector<NoAST*> filhos;

    NoAST(string t, string v, int l = 0)
        : tipo(t), valor(v), tipoSemantico(""), linha(l) {}

    void adicionarFilho(NoAST* filho) {
        filhos.push_back(filho);
    }

    // Impressão simples (Fase 2 — sem tipo semântico)
    void imprimirArvore(ostream& out, int nivel = 0) const {
        for (int i = 0; i < nivel; ++i) out << "  |";
        out << "-- [" << tipo << "] " << valor << "\n";
        for (auto filho : filhos)
            filho->imprimirArvore(out, nivel + 1);
    }

    // Impressão atribuída (Fase 3 — com tipo semântico)
    void imprimirArvoreAtribuida(ostream& out, int nivel = 0) const {
        for (int i = 0; i < nivel; ++i) out << "  |";
        out << "-- [" << tipo;
        if (!tipoSemantico.empty()) out << ":" << tipoSemantico;
        out << "] " << valor << "\n";
        for (auto filho : filhos)
            filho->imprimirArvoreAtribuida(out, nivel + 1);
    }
};

// Declaração da função do parser (construída em ParserAST.cpp)
// erros acumula mensagens sem abortar no primeiro erro (recuperação de erros)
NoAST* gerarArvore(vector<Token> tokens, vector<string>& erros);

#endif

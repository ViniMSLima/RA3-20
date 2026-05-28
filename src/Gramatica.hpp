// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#ifndef GRAMATICA_HPP
#define GRAMATICA_HPP

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
using namespace std;

// Definição do tipo da Gramática
typedef map<string, vector<vector<string>>> Gramatica;

// Variáveis globais — definidas em ParserLL1.cpp
extern Gramatica gramatica;
extern map<string, set<string>> conjuntosFirst;
extern map<string, set<string>> conjuntosFollow;
extern map<string, map<string, vector<string>>> tabelaLL1;

// Declarações das funções
void construirGramatica();
bool validarTerminal(string simbolo);
set<string> buscarFirst(string simbolo);
void calcularFirst();
void calcularFollow();
void construirTabelaLL1();

#endif

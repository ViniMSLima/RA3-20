// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#ifndef ANALISADOR_SEMANTICO_HPP
#define ANALISADOR_SEMANTICO_HPP

#include "AST.hpp"
#include "TabelaSimbolos.hpp"
#include <vector>
#include <string>
#include <map>
using namespace std;

// ============================================================
// As 5 funções obrigatórias da Fase 3 (conforme Seção 7 do edital)
// ============================================================

// 1. Prepara a entrada semântica: executa léxico, parser e valida (START)/(END).
//    Retorna {tokens, arvore_sintatica_base}.
//    Se houver erro léxico ou sintático, relata e retorna {vazio, nullptr}.
pair<vector<Token>, NoAST*> prepararEntradaSemantica(
    const string& arquivo,
    vector<string>& errosLex,
    vector<string>& errosSin
);

// 2. Percorre a AST e registra todas as variáveis: tipo, linha de definição,
//    linha de uso. Detecta uso antes de definição e redefinições incompatíveis.
TabelaSimbolos construirTabelaSimbolos(
    NoAST* arvore,
    vector<string>& errosSem
);

// 3. Valida os tipos de todas as expressões. Retorna um mapa de nó → tipo inferido.
//    Detecta incompatibilidades, operações inválidas e condições não booleanas.
map<NoAST*, string> verificarTipos(
    NoAST* arvore,
    TabelaSimbolos& ts,
    vector<string>& errosSem
);

// 4. Anota cada nó relevante da AST com tipoSemantico, produzindo a AST Atribuída.
//    Salva a árvore atribuída em arquivo Markdown.
NoAST* gerarArvoreAtribuida(
    NoAST* arvore,
    TabelaSimbolos& ts,
    map<NoAST*, string>& tipos
);

// 5. Gera código Assembly ARMv7 a partir da árvore atribuída.
//    SÓ é chamado quando não há nenhum erro léxico, sintático ou semântico.
void gerarAssembly(NoAST* arvoreAtribuida, const string& nomeArquivo);

#endif

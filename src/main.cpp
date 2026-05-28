// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#include "AnalisadorSemantico.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

// ============================================================
// Salva o relatório de erros semânticos em Markdown
// ============================================================
static void salvarRelatorioErros(
    const vector<string>& errosLex,
    const vector<string>& errosSin,
    const vector<string>& errosSem,
    const string& arquivoEntrada)
{
    ofstream out("erros_semanticos.md");
    if (!out.is_open()) return;

    out << "# Relatório de Erros\n\n";
    out << "**Arquivo analisado:** `" << arquivoEntrada << "`\n\n";

    // Léxicos
    out << "## Erros Léxicos\n\n";
    if (errosLex.empty())
        out << "_Nenhum erro léxico encontrado._\n\n";
    else
        for (const auto& e : errosLex) out << "- " << e << "\n";

    // Sintáticos
    out << "\n## Erros Sintáticos\n\n";
    if (errosSin.empty())
        out << "_Nenhum erro sintático encontrado._\n\n";
    else
        for (const auto& e : errosSin) out << "- " << e << "\n";

    // Semânticos
    out << "\n## Erros Semânticos\n\n";
    if (errosSem.empty())
        out << "_Nenhum erro semântico encontrado._\n\n";
    else
        for (const auto& e : errosSem) out << "- " << e << "\n";

    out.close();
    cout << "[ARTEFATO] Relatorio de erros salvo em: erros_semanticos.md" << endl;
}

// ============================================================
// MAIN — Pipeline completo do compilador RPN Fase 3
// Uso: ./AnalisadorSemantico <arquivo.txt>
// ============================================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "\n[ERRO FATAL] Arquivo de codigo-fonte nao informado!" << endl;
        cerr << "Modo de uso: ./AnalisadorSemantico <nome_do_arquivo.txt>" << endl;
        cerr << "Exemplo:     ./AnalisadorSemantico teste1.txt\n" << endl;
        return 1;
    }

    string arquivoCodigo = argv[1];

    cout << "=============================================" << endl;
    cout << " COMPILADOR RPN - ANALISADOR SEMANTICO" << endl;
    cout << " Arquivo: " << arquivoCodigo << endl;
    cout << "=============================================" << endl;

    vector<string> errosLex, errosSin, errosSem;

    // ──────────────────────────────────────────────
    // FASE 1 + 2: prepararEntradaSemantica
    // ──────────────────────────────────────────────
    auto [tokens, arvore] = prepararEntradaSemantica(arquivoCodigo, errosLex, errosSin);

    if (arvore == nullptr) {
        cerr << "\n[FALHA] Compilacao abortada: arvore sintatica nao pode ser construida." << endl;
        salvarRelatorioErros(errosLex, errosSin, errosSem, arquivoCodigo);
        return 1;
    }

    cout << "\n[FASE 2] AST base construida." << endl;

    // Imprime AST base (sem tipos) no console e em arquivo
    {
        cout << "\n--- Arvore Sintatica Base ---" << endl;
        arvore->imprimirArvore(cout);

        ofstream fArvore("arvore_sintatica.txt");
        if (fArvore.is_open()) {
            arvore->imprimirArvore(fArvore);
            fArvore.close();
            cout << "[ARTEFATO] Arvore sintatica salva em: arvore_sintatica.txt" << endl;
        }
    }

    // ──────────────────────────────────────────────
    // FASE 3A: construirTabelaSimbolos
    // ──────────────────────────────────────────────
    cout << "\n[FASE 3A] Construindo tabela de simbolos..." << endl;
    TabelaSimbolos ts = construirTabelaSimbolos(arvore, errosSem);

    cout << "[FASE 3A] " << ts.tabela.size() << " simbolo(s) registrado(s)." << endl;

    // ──────────────────────────────────────────────


    return 0;
}

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
    // FASE 3B: verificarTipos
    // ──────────────────────────────────────────────
    cout << "\n[FASE 3B] Verificando tipos..." << endl;
    auto tipos = verificarTipos(arvore, ts, errosSem);
    cout << "[FASE 3B] Verificacao concluida." << endl;

    // ──────────────────────────────────────────────
    // FASE 3C: gerarArvoreAtribuida
    // ──────────────────────────────────────────────
    cout << "\n[FASE 3C] Gerando arvore sintática atribuida..." << endl;
    NoAST* arvoreAtribuida = gerarArvoreAtribuida(arvore, ts, tipos);
    cout << "[FASE 3C] Arvore atribuida gerada." << endl;

    // Salva tabela de símbolos
    ts.salvarMarkdown("tabela_simbolos.md");
    ts.salvarJSON("tabela_simbolos.json");

    // Salva relatório de erros (mesmo que vazio)
    salvarRelatorioErros(errosLex, errosSin, errosSem, arquivoCodigo);

    // ──────────────────────────────────────────────
    // Verifica se pode gerar Assembly
    // ──────────────────────────────────────────────
    bool semErros = errosLex.empty() && errosSin.empty() && errosSem.empty();

    if (!semErros) {
        cout << "\n=============================================" << endl;
        cout << " COMPILACAO COM ERROS — ASSEMBLY NAO GERADO" << endl;
        cout << "=============================================" << endl;

        int total = (int)(errosLex.size() + errosSin.size() + errosSem.size());
        cout << "Total de erros: " << total << endl;
        cout << "  Lexicos:    " << errosLex.size() << endl;
        cout << "  Sintaticos: " << errosSin.size() << endl;
        cout << "  Semanticos: " << errosSem.size() << endl;
        cout << "\nConsulte 'erros_semanticos.md' para detalhes." << endl;
        return 1;
    }

    // ──────────────────────────────────────────────
    // FASE 4: gerarAssembly (somente se sem erros)
    // ──────────────────────────────────────────────
    cout << "\n[FASE 4] Gerando codigo Assembly ARMv7..." << endl;
    gerarAssembly(arvoreAtribuida, "saida.s");

    // ──────────────────────────────────────────────
    // Resumo final
    // ──────────────────────────────────────────────
    cout << "\n=============================================" << endl;
    cout << " COMPILACAO CONCLUIDA COM SUCESSO!" << endl;
    cout << "=============================================" << endl;
    cout << "Artefatos gerados:" << endl;
    cout << "  - arvore_sintatica.txt  (AST base)" << endl;
    cout << "  - arvore_atribuida.md   (AST com tipos)" << endl;
    cout << "  - tabela_simbolos.md    (tabela de simbolos)" << endl;
    cout << "  - tabela_simbolos.json  (tabela JSON)" << endl;
    cout << "  - erros_semanticos.md   (relatorio de erros)" << endl;
    cout << "  - saida.s               (codigo Assembly ARMv7)" << endl;


    return 0;
}

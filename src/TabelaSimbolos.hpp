// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#ifndef TABELA_SIMBOLOS_HPP
#define TABELA_SIMBOLOS_HPP

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

// Entrada na Tabela de Símbolos
struct SimboloEntry {
    string nome;           // Identificador da variável (ex: "X", "CONTADOR")
    string tipo;           // Tipo inferido: "int", "real", "bool", "" = desconhecido
    int    linhaDefinicao; // Linha onde foi definida via (V MEM)
    int    linhaUltimoUso; // Linha do último uso registrado
    bool   inicializada;   // true somente após (V MEM)
};

// Tabela de Símbolos
class TabelaSimbolos {
public:
    map<string, SimboloEntry> tabela;

    // Registra uma nova variável ou atualiza tipo se já existir
    // Retorna false se já existia com tipo diferente (redefinição incompatível)
    bool definir(const string& nome, const string& tipo, int linha) {
        if (tabela.count(nome) && tabela[nome].inicializada) {
            if (!tabela[nome].tipo.empty() && tabela[nome].tipo != tipo)
                return false; // redefinição com tipo diferente
        }
        tabela[nome] = {nome, tipo, linha, linha, true};
        return true;
    }

    bool existe(const string& nome) const {
        return tabela.count(nome) > 0;
    }

    bool estaInicializada(const string& nome) const {
        if (!existe(nome)) return false;
        return tabela.at(nome).inicializada;
    }

    string getTipo(const string& nome) const {
        if (!existe(nome)) return "";
        return tabela.at(nome).tipo;
    }

    void setTipo(const string& nome, const string& tipo) {
        if (existe(nome)) tabela[nome].tipo = tipo;
    }

    void registrarUso(const string& nome, int linha) {
        if (existe(nome)) tabela[nome].linhaUltimoUso = linha;
    }

    // Salva a tabela em formato Markdown
    void salvarMarkdown(const string& nomeArquivo) const {
        ofstream out(nomeArquivo);
        if (!out.is_open()) {
            cerr << "[ERRO] Nao foi possivel salvar a tabela de simbolos em: "
                 << nomeArquivo << endl;
            return;
        }

        out << "# Tabela de Símbolos\n\n";
        out << "| Nome | Tipo | Linha Definição | Linha Último Uso | Inicializada |\n";
        out << "|------|------|-----------------|------------------|--------------|\n";

        for (const auto& [nome, e] : tabela) {
            out << "| " << e.nome
                << " | "  << (e.tipo.empty() ? "?" : e.tipo)
                << " | "  << e.linhaDefinicao
                << " | "  << e.linhaUltimoUso
                << " | "  << (e.inicializada ? "Sim" : "Não")
                << " |\n";
        }

        out.close();
        cout << "[ARTEFATO] Tabela de simbolos salva em: " << nomeArquivo << endl;
    }

    // Salva a tabela em formato JSON
    void salvarJSON(const string& nomeArquivo) const {
        ofstream out(nomeArquivo);
        if (!out.is_open()) return;

        out << "{\n  \"tabela_simbolos\": [\n";
        bool primeiro = true;
        for (const auto& [nome, e] : tabela) {
            if (!primeiro) out << ",\n";
            out << "    {\n"
                << "      \"nome\": \""           << e.nome           << "\",\n"
                << "      \"tipo\": \""            << (e.tipo.empty() ? "?" : e.tipo) << "\",\n"
                << "      \"linha_definicao\": "   << e.linhaDefinicao << ",\n"
                << "      \"linha_ultimo_uso\": "  << e.linhaUltimoUso << ",\n"
                << "      \"inicializada\": "      << (e.inicializada ? "true" : "false") << "\n"
                << "    }";
            primeiro = false;
        }
        out << "\n  ]\n}\n";
        out.close();
        cout << "[ARTEFATO] Tabela de simbolos salva em: " << nomeArquivo << endl;
    }
};

#endif

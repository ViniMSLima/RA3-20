// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#include "Fase1_Lexico.hpp"
#include <fstream>
#include <sstream>
#include <cctype>

// Verifica se todos os caracteres de uma palavra são letras latinas maiúsculas (A-Z)
static bool todosMaiusculos(const string& palavra) {
    for (char c : palavra) {
        if (c < 'A' || c > 'Z') return false;
    }
    return true;
}

// Verifica se uma palavra é um número válido (inteiro ou real)
// Ex: "42", "3.14", ".5" são válidos; "3.1.4" não é
static bool ehNumeroValido(const string& palavra) {
    bool temPonto = false;
    for (size_t i = 0; i < palavra.size(); i++) {
        if (palavra[i] == '.') {
            if (temPonto) return false; // dois pontos = inválido
            temPonto = true;
        } else if (!isdigit(palavra[i])) {
            return false;
        }
    }
    return !palavra.empty();
}

// Verifica se a palavra é um operador ou parêntese reconhecido
static bool ehOperadorOuParentese(const string& palavra) {
    return (palavra == "(" || palavra == ")" ||
            palavra == "+" || palavra == "-" ||
            palavra == "*" || palavra == "/" ||
            palavra == "|" || palavra == "%" ||
            palavra == "^" || palavra == ">" ||
            palavra == "<" || palavra == "=");
}

bool executarFase1(string arquivoEntrada, string arquivoSaida) {
    ifstream arquivo(arquivoEntrada);
    ofstream saida(arquivoSaida);

    if (!arquivo.is_open()) {
        cerr << "[ERRO FASE 1] Nao foi possivel abrir o arquivo de entrada: "
             << arquivoEntrada << endl;
        return false;
    }
    if (!saida.is_open()) {
        cerr << "[ERRO FASE 1] Nao foi possivel criar o arquivo de saida: "
             << arquivoSaida << endl;
        return false;
    }

    bool semErros    = true;
    bool emComentario = false; // controla se estamos dentro de *{ }*
    int  numeroLinha  = 1;

    string linhaTexto;
    while (getline(arquivo, linhaTexto)) {
        // Percorremos caractere a caractere para tratar comentários corretamente
        // Primeiro montamos a linha "limpa" removendo os trechos de comentário
        string linhaLimpa;
        size_t i = 0;
        while (i < linhaTexto.size()) {
            if (emComentario) {
                // Procura pelo fechamento }*
                if (i + 1 < linhaTexto.size() &&
                    linhaTexto[i] == '}' && linhaTexto[i+1] == '*') {
                    emComentario = false;
                    i += 2; // pula '}' e '*'
                } else {
                    i++; // ignora o caractere dentro do comentário
                }
            } else {
                // Procura pela abertura *{
                if (i + 1 < linhaTexto.size() &&
                    linhaTexto[i] == '*' && linhaTexto[i+1] == '{') {
                    emComentario = true;
                    i += 2; // pula '*' e '{'
                } else {
                    linhaLimpa += linhaTexto[i];
                    i++;
                }
            }
        }

        // Se ainda está em comentário e acabou a linha, continua na próxima
        // (comentário multi-linha), nada a tokenizar nessa linha

        // Agora tokenizamos a linhaLimpa
        stringstream ss(linhaLimpa);
        string palavra;

        while (ss >> palavra) {
            string tipo;

            // --- Classificação ---

            if (ehOperadorOuParentese(palavra)) {
                if (palavra == "(" || palavra == ")")
                    tipo = "PARENTE";
                else
                    tipo = "OPERADOR";
            }
            else if (palavra == "START" || palavra == "END"   ||
                     palavra == "IF"    || palavra == "WHILE" ||
                     palavra == "MEM"   || palavra == "RES"   ||
                     palavra == "true"  || palavra == "false") {
                tipo = "KEYWORD";
            }
            else if (isdigit(palavra[0]) || palavra[0] == '.') {
                if (ehNumeroValido(palavra)) {
                    tipo = "NUMERO";
                } else {
                    cerr << "[ERRO LEXICO] Linha " << numeroLinha
                         << ": Numero malformado '" << palavra << "'" << endl;
                    semErros = false;
                    continue; // descarta token inválido
                }
            }
            else if (isupper(palavra[0]) && todosMaiusculos(palavra)) {
                tipo = "VARIAVEL";
            }
            else {
                // Token inválido — não é dígito, letra maiúscula, operador ou keyword
                cerr << "[ERRO LEXICO] Linha " << numeroLinha
                     << ": Token invalido '" << palavra << "'" << endl;
                semErros = false;
                continue; // descarta, NÃO escreve no arquivo de saída
            }

            // Formato: TIPO,VALOR,LINHA
            saida << tipo << "," << palavra << "," << numeroLinha << "\n";
        }

        numeroLinha++;
    }

    // Verifica se ficou comentário aberto sem fechar
    if (emComentario) {
        cerr << "[ERRO LEXICO] Comentario '*{' aberto nunca foi fechado com '}*'" << endl;
        semErros = false;
    }

    arquivo.close();
    saida.close();

    if (semErros)
        cout << "[FASE 1] Analise Lexica concluida sem erros." << endl;
    else
        cout << "[FASE 1] Analise Lexica concluida COM erros lexicos." << endl;

    return semErros;
}

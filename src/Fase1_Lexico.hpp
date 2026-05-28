// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#ifndef FASE1_LEXICO_HPP
#define FASE1_LEXICO_HPP

#include <iostream>
#include <string>
using namespace std;

// Executa a análise léxica do arquivo de entrada.
// Reconhece e DESCARTA comentários delimitados por *{ e }*
// Rejeita tokens inválidos (não são letras latinas maiúsculas, dígitos ou símbolos conhecidos).
// Retorna true se não houve nenhum erro léxico, false caso contrário.
bool executarFase1(string arquivoEntrada, string arquivoSaida);

#endif

// Integrantes do grupo (ordem alfabética):
// Vinícius Matheus Sary de Lima - ViniMSLima
//
// Nome do grupo no Canvas: RA3_20

#include "AST.hpp"
#include "Gramatica.hpp"

// ============================================================
// PARSER LL(1) com construção de AST e RECUPERAÇÃO DE ERROS
//
// Correção da Fase 2 (R4): ao encontrar um erro sintático,
// em vez de retornar nullptr imediatamente, o parser registra
// o erro e avança os tokens até um ponto de sincronização
// (PARENTE_ESQ ou EOF), permitindo continuar a análise e
// detectar múltiplos erros em uma única execução.
// ============================================================

NoAST* gerarArvore(vector<Token> tokens, vector<string>& erros) {
    stack<string>  pilhaSintatica;
    stack<NoAST*>  pilhaSemantica;

    // Regra de Ouro do LL(1): EOF embaixo, símbolo inicial em cima
    pilhaSintatica.push("EOF");
    pilhaSintatica.push("programa");

    // Marcador de fim de fita
    Token tokenEOF;
    tokenEOF.tipo  = "EOF";
    tokenEOF.valor = "EOF";
    tokenEOF.linha = -1;
    tokens.push_back(tokenEOF);

    int ponteiro = 0;
    int maxTokens = (int)tokens.size();

    auto avancarAteSincronizacao = [&]() {
        // Modo pânico: avança até PARENTE_ESQ ou EOF
        while (ponteiro < maxTokens &&
               tokens[ponteiro].tipo != "PARENTE_ESQ" &&
               tokens[ponteiro].tipo != "EOF") {
            ponteiro++;
        }
        // Limpa a pilha sintática até um não-terminal que aceite PARENTE_ESQ
        while (!pilhaSintatica.empty() && pilhaSintatica.top() != "EOF") {
            string t = pilhaSintatica.top();
            if (!validarTerminal(t) && t != "EPSILON") break;
            pilhaSintatica.pop();
        }
    };

    while (!pilhaSintatica.empty()) {
        if (ponteiro >= maxTokens) break;

        string topo       = pilhaSintatica.top();
        Token  tokenAtual = tokens[ponteiro];

        // EPSILON não consome nada
        if (topo == "EPSILON") {
            pilhaSintatica.pop();
            continue;
        }

        // ── Terminal: deve casar com o token atual ──────────────────────
        if (validarTerminal(topo) || topo == "EOF") {
            if (topo == tokenAtual.tipo) {
                pilhaSintatica.pop();

                // ========================================================
                // CONSTRUÇÃO DA AST (ações semânticas embutidas no parser)
                // ========================================================

                // Folhas: literais numéricos
                if (topo == "NUMERO") {
                    pilhaSemantica.push(new NoAST("NUMERO", tokenAtual.valor, tokenAtual.linha));
                }
                // Folhas: literais booleanos (true/false)
                else if (topo == "BOOLEANO") {
                    pilhaSemantica.push(new NoAST("BOOLEANO", tokenAtual.valor, tokenAtual.linha));
                }
                // Folhas: variáveis
                else if (topo == "VARIAVEL") {
                    pilhaSemantica.push(new NoAST("VARIAVEL", tokenAtual.valor, tokenAtual.linha));
                }
                // Operadores binários aritméticos e relacionais
                else if (topo == "OP_ARITMETICO" || topo == "OP_RELACIONAL") {
                    NoAST* noOp = new NoAST(topo, tokenAtual.valor, tokenAtual.linha);
                    if (pilhaSemantica.size() >= 2) {
                        NoAST* dir = pilhaSemantica.top(); pilhaSemantica.pop();
                        NoAST* esq = pilhaSemantica.top(); pilhaSemantica.pop();
                        noOp->adicionarFilho(esq);
                        noOp->adicionarFilho(dir);
                    }
                    pilhaSemantica.push(noOp);
                }
                // IF e WHILE: filhos[0]=condição, filhos[1]=ação
                else if (topo == "KEY_IF" || topo == "KEY_WHILE") {
                    NoAST* noCmd = new NoAST(topo, tokenAtual.valor, tokenAtual.linha);
                    if (pilhaSemantica.size() >= 2) {
                        NoAST* acao     = pilhaSemantica.top(); pilhaSemantica.pop();
                        NoAST* condicao = pilhaSemantica.top(); pilhaSemantica.pop();
                        noCmd->adicionarFilho(condicao);
                        noCmd->adicionarFilho(acao);
                    }
                    pilhaSemantica.push(noCmd);
                }
                // MEM: filhos[0]=valor a armazenar, filhos[1]=nome da variável destino
                else if (topo == "KEY_MEM") {
                    NoAST* noMem = new NoAST("KEY_MEM", tokenAtual.valor, tokenAtual.linha);
                    if (pilhaSemantica.size() >= 2) {
                        NoAST* destino = pilhaSemantica.top(); pilhaSemantica.pop();
                        NoAST* valor   = pilhaSemantica.top(); pilhaSemantica.pop();
                        noMem->adicionarFilho(valor);    // filhos[0] = expressão de valor
                        noMem->adicionarFilho(destino);  // filhos[1] = variável de destino
                    } else if (!pilhaSemantica.empty()) {
                        noMem->adicionarFilho(pilhaSemantica.top());
                        pilhaSemantica.pop();
                    }
                    pilhaSemantica.push(noMem);
                }
                // RES: único filho = o número de linhas anteriores
                else if (topo == "KEY_RES") {
                    NoAST* noRes = new NoAST("KEY_RES", tokenAtual.valor, tokenAtual.linha);
                    if (!pilhaSemantica.empty()) {
                        noRes->adicionarFilho(pilhaSemantica.top());
                        pilhaSemantica.pop();
                    }
                    pilhaSemantica.push(noRes);
                }
                // Parênteses, START e END são estruturais — não viram nós na AST

                ponteiro++; // avança a fita
            } else {
                // ── ERRO: terminal esperado não encontrado ──────────────
                erros.push_back(
                    "[ERRO SINTATICO] Linha " + to_string(tokenAtual.linha) +
                    ": Esperava '" + topo + "' mas recebeu '" +
                    tokenAtual.tipo + "' (\"" + tokenAtual.valor + "\")"
                );
                cerr << erros.back() << endl;

                // Recuperação: sincronizar nos pontos estáveis
                pilhaSintatica.pop(); // descarta o terminal esperado
                avancarAteSincronizacao();
            }
        }
        // ── Não-Terminal: consulta a Tabela LL(1) ───────────────────────
        else {
            vector<string> regra = tabelaLL1[topo][tokenAtual.tipo];

            if (regra.empty()) {
                erros.push_back(
                    "[ERRO SINTATICO] Linha " + to_string(tokenAtual.linha) +
                    ": Nenhuma regra para [" + topo + "] com token [" +
                    tokenAtual.tipo + " \"" + tokenAtual.valor + "\"]"
                );
                cerr << erros.back() << endl;

                // Recuperação: descarta o não-terminal e sincroniza
                pilhaSintatica.pop();
                avancarAteSincronizacao();
                continue;
            }

            pilhaSintatica.pop();
            if (regra[0] != "EPSILON") {
                for (int i = (int)regra.size() - 1; i >= 0; i--)
                    pilhaSintatica.push(regra[i]);
            }
        }
    }

    // ── Monta o nó raiz com todos os comandos em ordem ──────────────────
    NoAST* raiz = new NoAST("RAIZ", "PROGRAMA_COMPLETO");

    vector<NoAST*> temp;
    while (!pilhaSemantica.empty()) {
        temp.push_back(pilhaSemantica.top());
        pilhaSemantica.pop();
    }
    for (int i = (int)temp.size() - 1; i >= 0; i--)
        raiz->adicionarFilho(temp[i]);

    return raiz;
}

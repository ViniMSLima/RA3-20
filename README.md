# Compilador RPN — Fase 3 (Analisador Semântico)

---

## 🏛️ Informações Institucionais

| Campo | Valor |
|-------|-------|
| **Instituição** | Pontifícia Universidade Católica do Paraná (PUC-PR) |
| **Curso** | Engenharia de Computação |
| **Período** | 9º Período — 2026 |
| **Disciplina** | Linguagens Formais e Compiladores (LFC) |
| **Professor** | Frank de Alcantara |

---

## 👥 Grupo

| Nome | GitHub | Função |
|------|--------|--------|
| Vinícius Matheus Sary de Lima | [@ViniMSLima](https://github.com/ViniMSLima) | Administrador |

**Nome do grupo no Canvas:** RA3_20

---

## 🛠️ Compilação, Execução e Depuração

### Compilação

**Via script (Windows):**
```powershell
.\compilar.bat
```

**Via terminal (manual):**
```powershell
g++ -std=c++17 -Wall -Wextra -Isrc src/Fase1_Lexico.cpp src/LexerInterface.cpp src/ParserLL1.cpp src/ParserAST.cpp src/AnalisadorSemantico.cpp src/main.cpp -o AnalisadorSemantico.exe
```

### Execução

```powershell
.\AnalisadorSemantico.exe teste1.txt
.\AnalisadorSemantico.exe teste2.txt
.\AnalisadorSemantico.exe teste3.txt
```

### Depuração

**Compilar com símbolos de depuração:**
```powershell
g++ -std=c++17 -g -Isrc src/Fase1_Lexico.cpp src/LexerInterface.cpp src/ParserLL1.cpp src/ParserAST.cpp src/AnalisadorSemantico.cpp src/main.cpp -o AnalisadorSemantico_debug.exe
```

**Depurar com GDB:**
```powershell
gdb .\AnalisadorSemantico_debug.exe
# Dentro do GDB:
(gdb) run teste1.txt
(gdb) bt              # backtrace em caso de crash
(gdb) break main      # breakpoint na main
(gdb) step            # executa linha a linha
```

**Verificar vazamentos de memória com Valgrind (Linux/WSL):**
```bash
valgrind --leak-check=full ./AnalisadorSemantico teste1.txt
```

---

## 📖 Descrição da Linguagem

A linguagem é baseada em **Notação Polonesa Reversa (RPN)** — os operandos precedem o operador.
Todo programa é delimitado por `( START )` e `( END )`.

### Estrutura Básica

```
( START )
( operando1 operando2 operador )
( END )
```

### Tipos Suportados

| Tipo | Descrição | Exemplos de Literais |
|------|-----------|----------------------|
| `int` | Inteiro (sem ponto decimal) | `0`, `42`, `100` |
| `real` | Real de precisão dupla (IEEE 754) | `3.14`, `2.5`, `0.0` |
| `bool` | Lógico | `true`, `false` |

### Operadores

| Operador | Nome | Tipos aceitos | Tipo do resultado |
|----------|------|---------------|-------------------|
| `+` | Adição | int/real × int/real | int ou real |
| `-` | Subtração | int/real × int/real | int ou real |
| `*` | Multiplicação | int/real × int/real | int ou real |
| `\|` | Divisão real | int/real × int/real | real |
| `/` | Divisão inteira | **int × int apenas** | int |
| `%` | Resto | **int × int apenas** | int |
| `^` | Potenciação | int/real × int/real | int ou real |
| `>` | Maior que | numérico × numérico | bool |
| `<` | Menor que | numérico × numérico | bool |
| `=` | Igual | numérico × numérico | bool |

### Comandos Especiais

| Comando | Descrição |
|---------|-----------|
| `( V MEM )` | Armazena o valor `V` na variável `MEM` (define o tipo) |
| `( MEM )` | Lê o valor da variável `MEM` |
| `( N RES )` | Retorna o resultado da instrução `N` posições anteriores |

### Estruturas de Controle

```
( ( Condição ) ( Ação ) IF )
( ( Condição ) ( Ação ) WHILE )
```
A condição **deve** ser do tipo `bool`.

### Comentários

```
*{ Comentário em linha inteira }*
( 10 X MEM ) *{ Comentário no final de linha }*
( X *{ Comentário entre expressões }* 5 + )
```

### Regras para Variáveis

1. Toda variável **deve** ser definida com `(V MEM)` antes de ser usada em expressões.
2. O tipo da variável é inferido no momento da definição e **não pode ser alterado**.
3. Nomes de variáveis: somente letras latinas maiúsculas (`A-Z`), sem dígitos ou símbolos.

---

## 🔍 Exemplos

### Programa Válido

```
( START )
( 10 X MEM )
( 3 Y MEM )
( ( X Y > ) ( ( X Y - ) Z MEM ) IF )
( ( Z 0 > ) ( ( Z 1 - ) Z MEM ) WHILE )
( END )
```

### Programa com Erros Semânticos

```
( START )
( X 5 + )         *{ ERRO: X usada antes de (V MEM) }*
( 10 N MEM )
( 2.5 R MEM )
( N R % )         *{ ERRO: % exige int, R é real }*
( ( N ) ( N 1 - ) WHILE )  *{ ERRO: condicao int, precisa de bool }*
( END )
```

---

## 📂 Artefatos Gerados

Após cada execução, os seguintes arquivos são gerados automaticamente:

| Arquivo | Conteúdo |
|---------|----------|
| `arvore_sintatica.txt` | Árvore Sintática base (sem tipos) |
| `arvore_atribuida.md` | Árvore Sintática Atribuída com tipos em cada nó |
| `tabela_simbolos.md` | Tabela de símbolos em Markdown |
| `tabela_simbolos.json` | Tabela de símbolos em JSON |
| `erros_semanticos.md` | Relatório de erros (léxicos, sintáticos e semânticos) |
| `saida.s` | Código Assembly ARMv7 (gerado apenas sem erros) |

---

## 📄 Documentação Técnica

| Documento | Localização |
|-----------|-------------|
| Gramática Atribuída (EBNF) | [`docs/GramaticaAtribuida.md`](docs/GramaticaAtribuida.md) |
| Regras de Tipos (Cálculo de Sequentes) | [`docs/RegrasTipos.md`](docs/RegrasTipos.md) |

---

## 🛡️ Robustez e Tratamento de Erros

- **Erros Léxicos:** tokens inválidos (ex: `@#`) são rejeitados com mensagem indicando linha e token.
- **Comentários:** `*{ }*` são reconhecidos e descartados antes da análise sintática.
- **Erros Sintáticos:** o parser continua após erros usando recuperação em modo pânico, reportando múltiplos erros.
- **Erros Semânticos:** mensagens claras indicam linha, variável/operador envolvido e causa.
- **Assembly:** gerado **somente** quando não há nenhum erro em nenhuma das três fases.

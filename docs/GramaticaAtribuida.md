# Gramática Atribuída — EBNF com Ações Semânticas

**Fase 3 — Analisador Semântico RPN**
**Grupo:** RA3_20 | **Autor:** Vinícius Matheus Sary de Lima

---

## Convenções

- Letras **minúsculas** → não-terminais
- Letras **MAIÚSCULAS** → terminais
- `{ ação }` → ação semântica executada durante a análise
- `τ` → tipo inferido de uma expressão

---

## Gramática

```ebnf
programa
    → PARENTE_ESQ START PARENTE_DIR proximo_bloco
      { validar_inicio_programa() }

proximo_bloco
    → PARENTE_ESQ corpo_instrucao
    | ε

corpo_instrucao
    → END PARENTE_DIR
      { validar_fim_programa() }
    | expressao PARENTE_DIR proximo_bloco

expressao
    → lista_operandos operador_final
      { τ(expressao) := inferirTipo(lista_operandos, operador_final) }

lista_operandos
    → operando lista_operandos
    | ε

operando
    → NUMERO
      { τ(operando) := "int" se sem ponto; "real" se com ponto }
    | BOOLEANO
      { τ(operando) := "bool" }
    | VARIAVEL
      { checar_declarada(VARIAVEL.nome);
        τ(operando) := lookup(tabela_simbolos, VARIAVEL.nome);
        registrar_uso(VARIAVEL.nome, VARIAVEL.linha) }
    | PARENTE_ESQ expressao PARENTE_DIR
      { τ(operando) := τ(expressao) }

operador_final
    → OP_ARITMETICO
      { checar_compatibilidade_aritmetica(τ_esq, τ_dir, OP_ARITMETICO.valor);
        τ(expressao) := regra_tipo_aritmetico(τ_esq, τ_dir, OP_ARITMETICO.valor) }

    | OP_RELACIONAL
      { checar_nao_bool(τ_esq, τ_dir, OP_RELACIONAL.valor);
        τ(expressao) := "bool" }

    | KEY_IF
      { checar_tipo(τ_condicao, "bool");
        τ(expressao) := τ_acao }

    | KEY_WHILE
      { checar_tipo(τ_condicao, "bool");
        τ(expressao) := τ_acao }

    | KEY_MEM
      { nome_var := ultimo_operando_variavel();
        checar_compatibilidade_tipo(nome_var, τ_valor);
        definir_variavel(tabela_simbolos, nome_var, τ_valor);
        τ(expressao) := τ_valor }

    | KEY_RES
      { n := valor_numerico(primeiro_operando());
        checar_indice_valido(n, instrucao_atual);
        τ(expressao) := "real" }
```

---

## Regras de Tipo para Operadores Aritméticos

| Operador | Operandos válidos | Tipo do resultado |
|----------|-------------------|-------------------|
| `+` `-` `*` `^` | int × int | int |
| `+` `-` `*` `^` | real × real | real |
| `+` `-` `*` `^` | int × real ou real × int | real |
| `/` `%` | int × int **apenas** | int |
| `\|` | int/real × int/real | real |
| `>` `<` `=` | numérico × numérico | bool |

**Operações com `bool` em qualquer posição → ERRO SEMÂNTICO.**

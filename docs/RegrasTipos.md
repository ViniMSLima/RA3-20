# Sistema de Regras de Tipos — Cálculo de Sequentes

**Fase 3 — Analisador Semântico RPN**
**Grupo:** RA3_20 | **Autor:** Vinícius Matheus Sary de Lima

---

## Notação

- `Γ` — contexto de tipos (tabela de símbolos)
- `Γ ⊢ e : τ` — no contexto Γ, a expressão `e` tem tipo `τ`
- `τ ∈ {int, real, bool}` — tipos suportados
- `x ∈ Γ` — variável `x` existe na tabela de símbolos
- `Γ(x)` — tipo de `x` na tabela

---

## 1. Regras para Literais

```
────────────────────────────────   (T-INT)
Γ ⊢ n : int          n é literal sem ponto decimal (ex: 42, 0, 100)


────────────────────────────────   (T-REAL)
Γ ⊢ r : real         r é literal com ponto decimal (ex: 3.14, 2.5, 0.0)


────────────────────────────────   (T-BOOL-TRUE)
Γ ⊢ true : bool


────────────────────────────────   (T-BOOL-FALSE)
Γ ⊢ false : bool
```

---

## 2. Regras para Variáveis

```
x ∈ Γ    x.inicializada = true    Γ(x) = τ
─────────────────────────────────────────────   (T-VAR)
Γ ⊢ (x) : τ


x ∉ Γ   OU   x.inicializada = false
───────────────────────────────────────────   (T-VAR-ERR)
ERRO: variável 'x' usada antes de ser definida com (V MEM)
```

---

## 3. Regras para Definição de Variáveis — MEM

```
Γ ⊢ e : τ    x ∉ Γ
──────────────────────────────────────────   (T-MEM-DEF)
Γ, x:τ ⊢ (e x MEM) : τ     define x com tipo τ


Γ ⊢ e : τ₂    x ∈ Γ    Γ(x) = τ₁    τ₁ = τ₂
──────────────────────────────────────────   (T-MEM-UPDATE)
Γ ⊢ (e x MEM) : τ₁     atualiza valor de x (tipo compatível)


Γ ⊢ e : τ₂    x ∈ Γ    Γ(x) = τ₁    τ₁ ≠ τ₂
──────────────────────────────────────────   (T-MEM-ERR)
ERRO: variável 'x' definida como τ₁ não pode receber valor de tipo τ₂
```

---

## 4. Regras para Operadores Aritméticos

```
Γ ⊢ e₁ : int    Γ ⊢ e₂ : int
─────────────────────────────────────────────   (T-ARITH-INT)
Γ ⊢ (e₁ e₂ op) : int        op ∈ {+, -, *, ^}


Γ ⊢ e₁ : real    Γ ⊢ e₂ : real
─────────────────────────────────────────────   (T-ARITH-REAL)
Γ ⊢ (e₁ e₂ op) : real       op ∈ {+, -, *, ^}


Γ ⊢ e₁ : int    Γ ⊢ e₂ : real
─────────────────────────────────────────────   (T-ARITH-MIXED-L)
Γ ⊢ (e₁ e₂ op) : real       op ∈ {+, -, *, ^}  (int promovido a real)


Γ ⊢ e₁ : real    Γ ⊢ e₂ : int
─────────────────────────────────────────────   (T-ARITH-MIXED-R)
Γ ⊢ (e₁ e₂ op) : real       op ∈ {+, -, *, ^}  (int promovido a real)


Γ ⊢ e₁ : τ₁    Γ ⊢ e₂ : τ₂    (τ₁ = bool OU τ₂ = bool)
─────────────────────────────────────────────────────────   (T-ARITH-BOOL-ERR)
ERRO: operação aritmética não pode ser usada com tipo 'bool'
```

---

## 5. Regras para Divisão Inteira e Resto

```
Γ ⊢ e₁ : int    Γ ⊢ e₂ : int
─────────────────────────────────────────────   (T-DIV-INT)
Γ ⊢ (e₁ e₂ /) : int


Γ ⊢ e₁ : int    Γ ⊢ e₂ : int
─────────────────────────────────────────────   (T-MOD-INT)
Γ ⊢ (e₁ e₂ %) : int


Γ ⊢ e₁ : τ₁    Γ ⊢ e₂ : τ₂    (τ₁ ≠ int OU τ₂ ≠ int)
──────────────────────────────────────────────────────   (T-DIV-ERR)
ERRO: operador '/' requer operandos do tipo 'int', recebeu τ₁ e τ₂


Γ ⊢ e₁ : τ₁    Γ ⊢ e₂ : τ₂    (τ₁ ≠ int OU τ₂ ≠ int)
──────────────────────────────────────────────────────   (T-MOD-ERR)
ERRO: operador '%' requer operandos do tipo 'int', recebeu τ₁ e τ₂
```

---

## 6. Regra para Divisão Real

```
Γ ⊢ e₁ : τ₁    Γ ⊢ e₂ : τ₂    τ₁,τ₂ ∈ {int, real}
─────────────────────────────────────────────────────   (T-RDIV)
Γ ⊢ (e₁ e₂ |) : real


Γ ⊢ e₁ : τ₁    Γ ⊢ e₂ : τ₂    (τ₁ = bool OU τ₂ = bool)
──────────────────────────────────────────────────────   (T-RDIV-ERR)
ERRO: operador '|' não pode ser usado com tipo 'bool'
```

---

## 7. Regras para Operadores Relacionais

```
Γ ⊢ e₁ : τ₁    Γ ⊢ e₂ : τ₂    τ₁,τ₂ ∈ {int, real}
─────────────────────────────────────────────────────   (T-REL)
Γ ⊢ (e₁ e₂ op) : bool       op ∈ {>, <, =}


Γ ⊢ e₁ : τ₁    Γ ⊢ e₂ : τ₂    (τ₁ = bool OU τ₂ = bool)
──────────────────────────────────────────────────────   (T-REL-ERR)
ERRO: operador relacional não pode ser aplicado ao tipo 'bool'
```

---

## 8. Regras para Estruturas de Controle

```
Γ ⊢ cond : bool    Γ ⊢ acao : τ
─────────────────────────────────   (T-IF)
Γ ⊢ (cond acao IF) : τ


Γ ⊢ cond : τ_c    τ_c ≠ bool
──────────────────────────────────────────────────────   (T-IF-ERR)
ERRO: condição do IF deve ser do tipo 'bool', recebeu τ_c


Γ ⊢ cond : bool    Γ ⊢ acao : τ
─────────────────────────────────   (T-WHILE)
Γ ⊢ (cond acao WHILE) : τ


Γ ⊢ cond : τ_c    τ_c ≠ bool
──────────────────────────────────────────────────────   (T-WHILE-ERR)
ERRO: condição do WHILE deve ser do tipo 'bool', recebeu τ_c
```

---

## 9. Regra para RES

```
n ∈ ℕ    n ≤ instrucao_atual
──────────────────────────────   (T-RES)
Γ ⊢ (n RES) : real      (resultado da instrução atual - n)


n > instrucao_atual
─────────────────────────────   (T-RES-WARN)
Aviso: índice RES fora do intervalo, retorna 0.0
```

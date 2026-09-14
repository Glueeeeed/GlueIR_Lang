# Control Flow & Conditions

GlueIR provides standard control flow constructs: branching with `if` / `else` statements and iteration with `while` loops.

---

## The `if` / `else` Statement

Conditional branching executes blocks of code based on a boolean condition:

```c
func int main() {
    int score = 85;

    if (score >= 50) {
        shout("Passed the exam!");
    } else {
        shout("Failed the exam.");
    }

    return 0;
}
```

---

## The `while` Loop

The `while` loop repeatedly executes a block as long as its condition evaluates to `true`:

```c
func int main() {
    int i = 0;

    while (i < 5) {
        shout("Current index: ", i);
        i = i + 1;
    }

    return 0;
}
```

---

## Operators & Precedence

GlueIR provides standard mathematical, relational, and boolean operators:

- **Arithmetic**: `+`, `-`, `*`, `/`
- **Comparison**: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Logical**: `and`, `or`, `!`, `&&`, `||`
- **Grouping**: Parentheses `(` `)`

### Operator Precedence
Evaluation follows standard algebraic and boolean rules:
1. Literals, identifiers, and parenthesized expressions `( ... )`
2. Multiplicative: `*`, `/`
3. Additive: `+`, `-`
4. Relational / Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
5. Logical AND: `and`, `&&`
6. Logical OR: `or`, `||`

::: tip Automatic Type Promotion
GlueIR automatically promotes mixed numerical operations:
- When an integer (`int`) interacts with a floating-point value (`float`/`double`), the integer is cast to float (`sitofp`).
- Operations between `float` and `double` promote the result to `double`.
:::


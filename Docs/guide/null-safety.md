# Null safety System

GlueIR introduces a **Null Safety** system centered around the `nulo` literal (the equivalent of `null`/`nil`). Its primary goal is to completely eliminate `NullPointerException`
style errors and undefined behavior at runtime.

The system draws inspiration from languages such as Kotlin, Swift, TypeScript, and C#.

---

## The `nulo` Literal

`nulo` is a keyword and literal that represents the absence of a value. The name is borrowed from **Spanish**, where *nulo* means "null" or "zero". It can be assigned to any **nullable** type.

```c
int? x = nulo;
string? name = nulo;
```

---

## Nullable Types

A type becomes a nullable type by adding the `?` modifier to its base type, creating the type **QPSN** (from the Spanish `Que puede ser nulo`).

| Nullable Type | Base Type | Description |
| :--- | :--- | :--- |
| `int?` | `int` | Integer that may hold `nulo` |
| `double?` | `double` | Double that may hold `nulo` |
| `string?` | `string` | String that may hold `nulo` |

::: warning Boolean Nullability
The `bool` type **cannot** be nullable. There is no `bool?` type in GlueIR a boolean value is always either `true` or `false`, never `nulo`.
:::



### Declaration Rules

- **Non-nullable types** must always be initialized at declaration. An uninitialized declaration (`int x;`) is a compile-time error.
- **Nullable types** without an explicit initializer are implicitly assigned `nulo`.

```c
int? x;        // Implicitly: int? x = nulo;
int y = 10;    // OK: initialized
// int z;      // Compile Error: non-nullable variable must be initialized
```

---

## Core Safety Rules

### Rule 1: No Assigning `nulo` to Non-Nullable Types

You cannot assign `nulo` or a nullable expression to a non-nullable variable.

```c
int a = 5;
a = nulo;      // Compile Error: cannot assign 'nulo' to non-nullable type 'int'

int? b = nulo;
a = b;         // Compile Error: cannot assign nullable 'int?' to non-nullable 'int'
```

### Rule 2: No Arithmetic Operations on Nullable Types

Arithmetic operators (`+`, `-`, `*`, `/`, `%`) cannot be applied to nullable types. The operand must be non-nullable.

```c
int? x = 10;
int? y = 20;

int result = x + y;  // Compile Error: cannot perform arithmetic on nullable types
```

To perform arithmetic, first narrow the type (see **Type Narrowing** below).

---

## Type Narrowing

Type Narrowing is a **flow-sensitive** mechanism. When the compiler encounters a condition that excludes `nulo`, it temporarily treats the variable as its non-nullable base type within the corresponding code block.

### Basic Narrowing with `!= nulo`

Inside the `then` branch of `if (x != nulo)`, the variable `x` is narrowed from `T?` to `T`:

```c
int? x = 10;

if (x != nulo) {
    int y = x;    // OK: x is narrowed to 'int' inside this block
    int sum = x + 5;  // OK: arithmetic is allowed
}

// Outside the block, x is still 'int?'
int z = x;        // Compile Error: x is 'int?' here
```

### Narrowing with `== nulo`

When checking `x == nulo`, the narrowing applies to the `else` branch:

```c
int? x;

if (x == nulo) {
    // x is still 'int?' here
} else {
    int y = x;    // OK: x is narrowed to 'int' in the else branch
}
```

### Implicit Truthiness

A nullable variable used directly as a condition narrows in the `then` branch, while negation (`!x`) narrows in the `else` branch:

```c
int? x = 10;

if (x) {
    int y = x;    // OK: narrowed to 'int'
}

if (!x) {
    // x is still nullable here
} else {
    int y = x;    // OK: narrowed to 'int'
}
```

---

## Edge Cases

### Reassignment Inside a Narrowed Block

If a narrowed variable is reassigned a nullable value inside the block, the narrowing is revoked:

```c
int? x = 10;

if (x != nulo) {
    int a = x;    // OK: x is 'int'
    x = nulo;     // x becomes nullable again
    int b = x;    // Compile Error: x is 'int?' now
}
```

### Early Exit / Guard Clauses

When a guard clause checks for `nulo` and exits early via `return`, `break`, or `throw`, the code after the `if` block inherits the narrowed state:

```c
func int calculate(int? x) {
    if (x == nulo) {
        return 0;
    }
    // x is narrowed to 'int' for the rest of the function
    return x + 5;
}
```

### `while` Loops

At the start of each iteration, the loop condition narrows the variable. Reassignment inside the body invalidates the narrowing for subsequent lines:

```c
int? x = 10;

while (x != nulo) {
    int a = x;    // OK: narrowed at the start of iteration
    x = getNext(); // May return nulo — narrowing invalidated
    int b = x;    // Compile Error: x is 'int?' again
}

// After the loop, x is always nulo (the loop terminated because x == nulo)
```

---

::: tip Design Philosophy
The Nulo system ensures that null-related errors are caught at **compile time**, not runtime. By making nullability explicit in the type system and narrowing types through flow analysis, GlueIR eliminates an entire class of bugs without requiring manual null checks throughout the codebase.
::::

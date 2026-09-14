# Functions & I/O

Functions, built-in helpers, and input/output form the core of program logic and interaction in GlueIR.

---

## Function Declarations & Parameters

Functions are declared using the `func` keyword. GlueIR supports parameter declarations, explicit return types, and default `void` return types:

```c
func int add(int a, int b) {
    return a + b;
}

// Omitting the return type defaults to void:
func printDivider() {
    shout("==========================");
}
```

### The `main` Entry Point

Every executable Glue program **must** contain an entry point function named `main` or `Main` with a return type of `int`.

```c
func int main() {
    printDivider();
    int sum = add(10, 25);
    shout("Result: ", sum);
    printDivider();
    return 0;
}
```

::: info Main Function Mechanics
At code generation time, the compiler automatically handles the `main` entry point:
- Initializes random seed (`srand`) based on current system time.
- Injects a completion message `Program completed successfully. Press Enter to exit.`.
- Injects a `getchar()` call to prevent immediate terminal termination on execution.
:::


GlueIR provides built-in functions for console input/output, random number generation, and type conversions:

### 1. Standard Output: `shout(...)`
A variadic function accepting multiple comma-separated expressions of any standard type:
```c
shout("Dog ", "Pedro", " is ", 4, " years old.");
```

### 2. Standard Input: `shin()`
Reads a string input from standard input:
```c
shout("Enter your name: ");
string name = shin();
shout("Welcome, ", name, "!");
```

### 3. Random Numbers: `random(min, max)`
Generates a random integer within the given inclusive range `[min, max]`:
```c
int rolled = random(1, 6);
shout("Dice roll: ", rolled);
```

### 4. String Parsing / Conversions
- `toInt(string)` - converts a string to an integer (`int`).
- `toFloat(string)` - converts a string to a single-precision float (`float`).
- `toDouble(string)` - converts a string to a double-precision float (`double`).

```c
shout("Enter an integer: ");
string rawInput = shin();
int parsedNumber = toInt(rawInput);
shout("Doubled: ", parsedNumber * 2);
```

---

## Current Function Limitations

::: info Function Scope & Order
1. **Sequential Declaration / Top-Level Definitions**:
   Functions should be declared before they are called, or defined at top-level.
2. **Scoping**:
   Function parameters and local variables are scoped to the function body.

:::
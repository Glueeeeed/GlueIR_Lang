# Compiler Architecture & LLVM Pipeline

The **Glue** compiler is a multi-phase, ahead-of-time (AOT) compiler that translates high-level Glue source code into LLVM Intermediate Representation (IR) and produces standalone native binaries using `clang`.

---

## Compilation Pipeline

```
                    ┌─────────────────────────┐
                    │    GlueIR Source Code     │
                    │        (*.glueir)         │
                    └────────────┬────────────┘
                                 │
                                 ▼
                    ┌─────────────────────────┐
                    │     Lexical Analyzer    │
                    │   (lexer.h / lexer.cpp) │
                    └────────────┬────────────┘
                                 │ Token Stream
                                 ▼
                    ┌─────────────────────────┐
                    │     Syntax Analyzer     │
                    │  (parser.h / parser.cpp)│
                    └────────────┬────────────┘
                                 │ Abstract Syntax Tree (AST)
                                 ▼
                    ┌─────────────────────────┐
                    │    Semantic Analyzer    │
                    │(semantic.h/semantic.cpp)│
                    └────────────┬────────────┘
                                 │ Verified AST
                                 ▼
                    ┌─────────────────────────┐
                    │   LLVM Code Generator   │
                    │ (codegen.h / codegen.cpp│
                    └────────────┬────────────┘
                                 │ LLVM IR (.ll)
                                 ▼
                    ┌─────────────────────────┐
                    │      Clang Backend      │
                    │ (Native Executable Binary│
                    └─────────────────────────┘
```

---

## Compiler Subsystems

### 1. Lexer (`../../src/lexer`)
- Scans raw character streams from input files.
- Generates structured `Token` instances (`TokenType`, `value`, `line`, `column`).
- Strips whitespace and single-line `//` comments.
- Custom tokenization of Glue block comments `/*g ... g*/`.
- Recognizes integer numbers, float literals (`12.5f`), double literals (`12.5`), strings, operators, and language keywords (`func`, `shout`, `const`, `sticky`, etc.).

### 2. Abstract Syntax Tree (`../../src/AST`)
- Represents program constructs in a hierarchical `ASTNode` graph.
- Node types include `PROGRAM`, `FUNCTION_DECLARATION`, `FUNCTION_CALL`, `DECLARATION`, `ASSIGNMENT`, `IF_STATEMENT`, `WHILE_STATEMENT`, `BINARY_OPERATION`, and primitive literal nodes.
- Preserves token line numbers and mutability flags (`isConst`, `isSticky`).

### 3. Parser (`../../src/parser`)
- Implements a recursive descent parser.
- Enforces grammar rules: top-level declarations, statement blocks, expressions with operator precedence hierarchy (Logical OR -> Logical AND -> Relational -> Additive -> Multiplicative -> Literals).
- Produces meaningful `ParseError` diagnostics with line and column pointers upon syntax errors.

### 4. Semantic Analyzer (`../../src/semantic`)
- Maintains a symbol table tracking identifiers, data types, and mutability states.
- Verifies that `main` exists, returns `int`, and terminates with a return statement.
- Enforces strict mutability rules:
  - Reassignment to `const` symbols is rejected at compile-time.
  - Reassignment to `sticky` symbols is permitted **exactly once** (`stickyUsed = true`), rejecting any second assignment.
- Verifies type compatibility across declarations and assignments.

### 5. LLVM Code Generator (`../../src/codegen`)
- Utilizes `llvm::LLVMContext`, `llvm::Module`, and `llvm::IRBuilder<>`.
- Creates native function entry points and handles stack memory via `AllocaInst`.
- Dynamically generates `printf` format strings and variable calls for `shout`.
- Generates conditional branches and phi nodes for `if`/`else` and `while` constructs.
- Emits formatted LLVM assembly (`glue.ll`) and invokes `clang` to produce the final executable.

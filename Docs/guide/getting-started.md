# Getting Started with Glue

Welcome to **Glue** is a minimalist, statically typed procedural programming language that compiles directly to native binaries via LLVM.

This guide walks you through installing dependencies, building the Glue compiler from source, and compiling your first program on **Linux**, **macOS**, and **Windows**.

---

## System Requirements

To build the Glue compiler, ensure you have the following installed on your machine:
- **C++ Compiler**: Supporting `C++20` standard (GCC 11+, Clang 13+, or MSVC 2019/2022)
- **CMake**: Version `3.20` or higher
- **LLVM & Clang**: Developer libraries, headers, and `clang` (LLVM 22 required)

::: warning
LLVM 22 is required for all features and optimizations in Glue.
Versions newer than 22 may be incompatible because LLVM drastically changes its API with each new version.
:::

---

## Installing Dependencies

::: code-group

```bash [macOS]
# Install LLVM 22 and CMake via Homebrew
brew install llvm@22 cmake

# CMake will automatically locate Homebrew LLVM using brew --prefix llvm@22
```

```bash [Linux (Ubuntu / Debian)]
# Install build tools, CMake, and LLVM/Clang 22
sudo apt update
sudo apt install -y build-essential cmake llvm-22-dev clang-22 libclang-22-dev
```

```bash [Linux (Fedora / RHEL)]
# Install build tools, CMake, and LLVM/Clang 22
sudo dnf install -y gcc-c++ cmake llvm22-devel clang22
```

```bash [Linux (Arch Linux)]
# Arch Linux natively tracks latest stable LLVM.
# To pin version 22, install it from AUR (e.g. using yay):
yay -S --needed base-devel cmake llvm22 clang22
```

```powershell [Windows]
# Instead of package managers, download the files directly to ensure all required components are included:

# 1. Go to the official LLVM releases page (https://github.com/llvm/llvm-project/releases/tag/llvmorg-22.1.8) and download the archive for version 22.1.8.

# 2. Extract the downloaded archive and move its entire contents to `C:\Program Files\LLVM`.



# Ensure Visual Studio with "Desktop development with C++" is installed.

# If LLVM is not in standard paths, set LLVM_DIR environment variable:

$env:LLVM_DIR = "C:\Program Files\LLVM\lib\cmake\llvm"
```

:::

---

## Building the Compiler

Clone the repository and compile the project using CMake across any platform:

::: code-group

```bash [Linux / macOS]
mkdir build && cd build
cmake ..
cmake --build .
```

```powershell [Windows (PowerShell / CMD)]
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

:::


::: info
 **Troubleshooting: LNK1181 Error (Windows)**

*If you encounter a linker error stating that it cannot open `diaguids.lib` at a specific Visual Studio path `(for example: C:\Program Files\Microsoft Visual Studio\2022\Enterprise\DIA SDK\lib\amd64\diaguids.lib)`, apply the following fix:*


**Locate the File:** *Search your current Visual Studio installation directory to find the actual location of the `diaguids.lib` file.*


**Recreate the Path:** *Manually create the missing folder structure* `(C:\Program Files\Microsoft Visual Studio\2022\Enterprise\DIA SDK\lib\amd64\)` *exactly as shown in the error message, and copy the `diaguids.lib` file into that directory to satisfy the linker path.*

:::

Upon a successful build, the `glue` (or `glue.exe` on Windows) compiler executable will be available in your build directory.

---

## Your First Glue Program

Create a source file named `main.glue`:

```c
func int main() {
    shout("Hello from Glue!");
    return 0;
}
```

### Compiling and Running

Invoke the compiler by passing your source file:

::: code-group

```bash [Linux / macOS]
./glue main.glue
```

```powershell [Windows]
.\glue.exe main.glue
```

:::

::: info
What happens under the hood?
1. The lexer and parser generate and validate the Abstract Syntax Tree (AST).
2. The semantic analyzer enforces types and mutability invariants.
3. The LLVM code generator emits Intermediate Representation into `glue.ll` tailored for the host target.
4. The compiler invokes `clang` to produce the native executable (`glue_program` or `glue_program.exe`) and runs it.
:::

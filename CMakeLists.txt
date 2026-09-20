cmake_minimum_required(VERSION 3.20)
project(GlueIR LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform-specific LLVM search paths
if(APPLE)
    # Automatically resolve Homebrew LLVM path (both Apple Silicon and Intel)
    execute_process(
            COMMAND brew --prefix llvm
            OUTPUT_VARIABLE HOMEBREW_LLVM_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    if(HOMEBREW_LLVM_PREFIX)
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_LLVM_PREFIX}/lib/cmake/llvm" "${HOMEBREW_LLVM_PREFIX}")
    endif()
    list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew/opt/llvm@22/lib/cmake/llvm"
        "/usr/local/opt/llvm@22/lib/cmake/llvm"
        "/opt/local/libexec/llvm"
    )
elseif(WIN32)
    # Common Windows LLVM installation paths (Winget, Chocolatey, Official Installer)
    if(DEFINED ENV{LLVM_DIR})
        list(APPEND CMAKE_PREFIX_PATH "$ENV{LLVM_DIR}")
    endif()
    if(DEFINED ENV{LLVM_ROOT})
        list(APPEND CMAKE_PREFIX_PATH "$ENV{LLVM_ROOT}")
    endif()
    list(APPEND CMAKE_PREFIX_PATH
        "C:/Program Files/LLVM/lib/cmake/llvm"
        "C:/Program Files/LLVM"
        "C:/LLVM/lib/cmake/llvm"
        "C:/LLVM"
    )
elseif(UNIX)
    # Common Linux LLVM installation paths
    if(DEFINED ENV{LLVM_DIR})
        list(APPEND CMAKE_PREFIX_PATH "$ENV{LLVM_DIR}")
    endif()
    list(APPEND CMAKE_PREFIX_PATH
        "/usr/lib/llvm-22/lib/cmake/llvm"
        "/usr/lib/llvm-22"
        "/usr/lib/llvm/lib/cmake/llvm"
        "/usr/lib/llvm"
    )
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "LLVM VERSION: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM libs: ${LLVM_LIBRARY_DIRS}")

add_definitions(${LLVM_DEFINITIONS})

add_executable(glue
        src/main.cpp
        src/lexer/lexer.cpp
        src/lexer/lexer.h
        src/parser/parser.cpp
        src/parser/parser.h
        src/AST/AST.cpp
        src/AST/ast.h
        src/semantic/semantic.cpp
        src/semantic/semantic.h
        src/codegen/codegen.cpp
        src/codegen/codegen.h
        src/semantic/scope.cpp
        src/semantic/scope.h
        src/parser/parser_utils.cpp
        src/parser/parser_statements.cpp
        src/semantic/semantic_utils.cpp
        src/codegen/codegen_utils.cpp
)

target_include_directories(glue PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${LLVM_INCLUDE_DIRS}
)

llvm_map_components_to_libnames(llvm_libs core orcjit native irreader support)

target_link_libraries(glue PRIVATE ${llvm_libs})

# Cross-platform compiler optimization flags
if(MSVC)
    target_compile_options(glue PRIVATE /O2 /EHsc)
else()
    target_compile_options(glue PRIVATE -O2)
endif()
---
layout: home

hero:
  name: "GlueIR Lang"
  text: "A Minimalist, procedural compiled Language"
  tagline: "High performance AOT compilation via LLVM with a streamlined syntax."
  actions:
    - theme: brand
      text: Get Started →
      link: /guide/getting-started
    - theme: alt
      text: Compiler Architecture
      link: /internals/compiler-architecture
    - theme: alt
      text: Code Examples
      link: /examples/code-samples

features:
  - title: LLVM-Powered Performance
    details: Source code is compiled ahead-of-time (AOT) to LLVM Intermediate Representation and optimized by Clang into native machine code.
  - title: Granular Mutability (sticky & const)
    details: Fine-grained variable state control with standard mutable variables, compile-time constants, and the unique sticky modifier (exactly one reassignment).
  - title:  Simple & Intuitive Syntax
    details: Clean C-family procedural syntax, static typing, built-in variadic shout logging, and custom block comments /*g ... g*/.
---

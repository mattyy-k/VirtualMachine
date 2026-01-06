# Virtual Machine

Welcome to my VM project!

This repository is currently in its early stages. I’ll be pushing updates regularly as I build out a fully custom virtual machine from scratch.

This README will evolve into full documentation for the VM as it becomes more complete.

## Virtual Machine Architecture (Design Phase)
### Overview

This document describes the architectural design and implementation plan for a stack-based virtual machine.
The project is currently in the design phase; implementation will begin after validating architectural decisions.

## Design Goals

Build a clear, minimal, and extensible virtual machine

Prioritize correctness and clarity over premature optimization

Design abstractions that allow future extensions (GC, closures, JIT)

## VM Architecture

### Execution Model

Stack-based virtual machine

Instruction Pointer (IP)

Operand stack

Heap for dynamically allocated objects

Central dispatch loop using opcode-based instruction decoding

Instruction Dispatch

Bytecode instruction stream

First byte represents opcode

Dispatch implemented via a switch-based loop

## Value Model

Dynamically typed

Single Value type using tagged-union semantics

Supports primitive values and references to heap-allocated objects

## Memory Model

Primitive values stored directly on the VM stack

Heap-allocated objects referenced via pointers in Value

No garbage collection in initial phase (planned to be implemented later)

## Planned Phases

### Phase 1 — Core VM

Bytecode format

Instruction dispatch

Arithmetic and control flow

### Phase 2 — Memory & Call Stack

Heap allocator

Stack frames and function calls

### Phase 3 — Garbage Collection

Mark-and-sweep collector

Root tracking

### Phase 4 — Parser & Compiler

Tokenizer

AST

Bytecode compiler

### Phase 5 — Advanced Features

Optimizations / JIT OR

Debugger & REPL OR

Closures and TCO

### Non-Goals (Initial Phases)

Garbage collection

Closures

JIT compilation

Native FFI

Multithreading

### Status

Phase 0 (Architecture & Design): Complete

Phase 1 (Implementation): Upcoming

If you’re interested in the day-to-day progress — design decisions, debugging disasters, breakthroughs, and lessons learned — check out the **progression.md** file.  
I’ll be treating it like a build log / dev diary for this project.

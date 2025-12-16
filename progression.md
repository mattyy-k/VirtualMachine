# progression.md — Build Log for My Virtual Machine
## This file is basically the VM’s diary. If something breaks, cries (me), explodes, or achieves sentience — it’s going in here.

## Day 0: Setup and Goals
### Why I'm building a VM: 
Because... Why Not? 😄 
But on a serious note, the project I built before this was a fully-functioning shell, and that really got me hooked to systems. The next logical step is building a Virtual Machine.
And more importantly, because I just really love building stuff.
### What features I want:
I plan to be as masochistic as possible, so short answer: All of them.
Long answer:

#### Phase 1 — Core VM (The Skeleton)
- A bytecode format (my VM’s own language, because why follow instructions?)
- Instruction dispatch loop (aka "the VM hamster wheel")
- A stack machine (push, pop, pray)
- Arithmetic ops (+, -, *, / — but without floating point meltdowns)
- Control flow: jumps, conditionals, loops

#### Phase 2 — Memory Model (Where objects go to live & occasionally die)
- Heap allocator
- Fixed layout for objects (integers, strings, arrays, etc.)
- Frames & call stack
- GC roots and tracing prep

#### Phase 3 — Garbage Collector (The Grim Reaper)
- Mark & Sweep (basic)
- Correct root finding (global vars, stack frames)
- No accidental deletion of living objects (hopefully)
- No memory leaks when GC forgets to sweep (hopefully again)

#### Phase 4 — Parser + Compiler (The VM learns to read)
- Tokenizer
- AST builder
- Compiler -> bytecode
- Functions, variables, local scopes
- Error messages that don’t gaslight users

#### Phase 5 — High-Level Language Features (The spice)
I'll add this as I want:

##### Option A — Performance Flex
- Bytecode optimizer
- Simple JIT (compile hot loops)
- Benchmark suite (compare with Lua/CPython)
- VM potentially gets faster than I do

##### Option B — Developer Experience (the “this is actually fun to code in” route)
- Debugger (step, breakpoints, inspect stack/heap)
- Error messages with line numbers & context
- REPL with syntax highlighting

##### Option C — Language Nerd Heaven (PL theory street cred)
- Closures with lexical scoping
- First-class functions
- Tail call optimization
- Theoretical bragging rights

### Languages/tools I'm using:
-C++
-VS Code
-Caffeine.
### What scares me/excites me:
Whatever scares me also excites me. 😈
### What I expect to learn:
Systems chops!
Also, how to function at 3 AM solely on caffeine fumes.

### Phase 0 (16/12/2025):
-Stack-based VM and not Register-based. Reason: fast enough, and much simpler to implement.

-Execution model:
  -The VM is a state machine whose state consists of the instruction pointer, operand stack and heap.
  -Instruction Pointer
  -Dispatch Loop (the heart of the VM)
  -Instruction Dispatch:
    -The first byte read is the opcode
    -The VM uses a giant switch statement to decode or dispatch the instruction.
    
-Value Model:
  -Dynamic typing
  -Value Type: single Value type with Tagged Union semantics.
  -Optimizations will not be implemented at this stage. Instead, I will design clean abstractions that allow future optimization.
  
-Memory Model:
  -Primitive values on the Vm stack; heap objects are referenced via pointers stored in Values.
-Non-goals: No GC, closures, JIT, threads, native FFI, Optimizer, Exceptions, YET. Will be implemented in later phases.
  

# sr

A compiler for a small statically typed systems language, written from scratch in C++.
It emits x86-64 assembly directly — no LLVM, no libc, no code generation library.
Programs link freestanding and talk to the kernel through raw syscalls.

```
fn fib(n: i32): i32 => {
    if n < 2 { ret n; }
    ret fib(n - 1) + fib(n - 2);
}

fn start(): i32 => { ret fib(10); }
```

```
$ ./Compiler fib.sr && ./build.sh && ./program; echo $?
55
```

## What's here

- **Hand-written lexer** — maximal-munch tokenisation, line/column tracking on every token.
- **Recursive-descent parser** — LL(1) grammar, one function per precedence level,
  panic-mode error recovery that keeps going after a syntax error instead of stopping
  at the first one.
- **Scope resolution** — lexical scope stack, shadowing, forward references for mutual
  recursion, immutability enforcement (`let` vs `mut`), stack frame layout.
- **Type checker** — bidirectional: types flow up from expressions, and expected types
  flow down so integer literals adopt their context (`let x: u8 = 65;` needs no cast).
- **Code generation** — direct x86-64 (Intel syntax, GAS), a custom stack-based calling
  convention, short-circuit logical operators, width-correct loads and stores.

Every pass reports errors with a line and column, and one bad expression produces
exactly one diagnostic rather than a cascade.

## The language

Full grammar in [`GRAMMAR.md`](GRAMMAR.md).

```
fn name(a: i32, mut b: [u8]): i32 => expression;   // expression body
fn name(): void => { ... }                         // block body

let x: i32 = 5;      // immutable
mut y = 5;           // mutable, type inferred
x = 6;               // error: x is not mut

i8 u8 i16 u16 i32 u32 i64 u64 bool void  [T]  [[T]]

if cond { } else if cond { } else { }    // no parens, mandatory braces
while cond { break; continue; }
value as u8                              // casts are explicit
a && b || !c                             // short-circuiting
arr[i][j] = v                            // arrays, nested, assignable
```

Conditions accept any non-void value, not just `bool`, so a null array is falsy.
Comparisons are non-associative: `a < b < c` is a syntax error rather than a
confusing parse.

## Building

Can use the run.sh script file to build and run automatically.

```
./run.sh test/example.sr
```

or can build from scratch.

```
cmake -B build && cmake --build build
```

Requires a C++17 compiler. No dependencies.

## Compiling and running a program

The compiler emits `output.s`. Linking it needs the runtime:

```sh
./build/Compiler program.sr
as output.s      -o output.o
as runtime.s     -o runtime_asm.o
gcc -c -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic -fno-pie \
    runtime.c -o runtime_c.o
ld output.o runtime_asm.o runtime_c.o -o program
./program
```

`-fno-pic -fno-pie` keeps gcc from emitting `@PLT` references, which break the
static link.

## Runtime

There is no libc. `runtime.s` implements `print` with a direct `write` syscall and
obtains memory from `mmap`. `runtime.c` holds a free-list allocator compiled
freestanding.

Because the compiler uses a custom stack-passing convention and freestanding C uses
System V, small assembly shims bridge the two:

```
codegen  ->  _malloc (stack args)  ->  malloc (C)  ->  get_memory  ->  mmap
```

Each arrow crosses exactly one convention boundary.

## Calling convention

Not System V. Arguments are pushed right to left and read at `[rbp+16]`, `[rbp+24]`,
and so on; the caller cleans up with `add rsp, N`. Stack frames are rounded to 16
bytes, and odd argument counts are padded, so calls into the freestanding C runtime
stay aligned.

Locals and parameters occupy a full 8-byte slot regardless of declared type. Only
packed array elements use their element width — a `[u8]` has stride 1 and loads with
`movzx`. That asymmetry is deliberate and is the single most important invariant in
the backend.

## Tests

```sh
./build/Compiler test/pass.sr        # must report zero errors
./build/Compiler test/scope_fail.sr  # 15 scope errors, one per marked line
./build/Compiler test/type_fail.sr   # 16 type errors, one per marked line
./build/Compiler test/example.sr     # compiles and prints its own results
```

`runtest.sr` is the end-to-end check: it computes arithmetic, recursion, loops,
array access, and casts, and prints each result with its expected value listed at
the bottom of the file.

## Editor support

`editor/nvim/` has a Vim syntax file for `.sr`.

## Status

Working: the full pipeline, all language features listed above, and a first-fit
free-list allocator.

Next: allocator block splitting and coalescing; integer-to-string in the runtime
rather than in user code; block comments.

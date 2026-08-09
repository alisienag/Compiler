# A Compiler (working title)

A small statically-typed language that compiles **directly to x86-64 Linux assembly** — no LLVM, no libc, no runtime. The compiler emits Intel-syntax assembly (assembled with `gas`), links to a freestanding static ELF, and talks to the kernel through raw syscalls.

The language currently supports three types (`i32`, `bool`, `string`), functions with recursion, lexical scoping with shadowing, `if`/`else if`/`else`, comparison and arithmetic operators, and a built-in `print` backed by the `write` syscall.

---

## Examples

### 1. Recursion — Fibonacci

Computes the (n+1)-th Fibonacci number (with `fib(0) = fib(1) = 1`, so the sequence is `1, 1, 2, 3, 5, …`).

```rust
fibo(i: i32): i32 => {
    if i == 0 {
        ret 1;
    } else if i == 1 {
        ret 1;
    } else {
        ret fibo(i - 1) + fibo(i - 2);
    }
}

main(): i32 => {
    ret fibo(10);
}
```

Exits with status code `89`.

### 2. Lexical scoping with shadowing

An inner block can declare a variable that shadows an outer one; the shadow is confined to its block and the outer variable is restored on exit.

```rust
main(): i32 => {
    let x: i32 = 1;
    {
        let x: i32 = 10;   // shadows the outer x, only inside this block
    }
    ret x;                 // 1 — the inner x did not leak
}
```

Exits with status code `1`.

### 3. Strings and the `print` syscall

`print` takes a string and an explicit byte length, and writes it to stdout via the `write` syscall. Reassignment and block scoping interact as expected.

```rust
main(): i32 => {
    let message: string = "hey\n";
    print(message, 4);          // hey
    {
        let message: string = "hello\n";
        print(message, 6);      // hello
    }
    print(message, 4);          // hey  (inner message is gone)
    message = "hi\n";
    print(message, 3);          // hi
    ret 0;
}
```

Output:

```
hey
hello
hey
hi
```

Exits with status code `0`.

### 4. Computed arguments and booleans

Arguments are full expressions, so a call can be passed as another call's argument. Comparisons produce `bool` values.

```rust
add(i: i32, j: i32): i32 => ret i + j;

main(): i32 => {
    let ready: bool = add(1, 1) == 2;
    if ready {
        print("go\n", 3);       // go
    }
    ret add(5, 10);
}
```

Exits with status code `15`.

> **Note:** `print` lengths are byte counts. `"hey\n"` is 4 bytes (`\n` is one byte); adjust the length if you change the string.

---

## Generated x86-64 assembly

The compiler emits Intel-syntax assembly for Linux, assembled with `gas`. Here is the actual output for the Fibonacci example above — recursion, both base-case branches, 16-byte stack alignment, and argument cleanup are all visible:

```asm
.intel_syntax noprefix
.section .data
.section .text
.globl main
print:
    push rbp
    mov rbp, rsp
    mov rax, 1              # write syscall
    mov rdi, 1              # fd = stdout
    mov rsi, [rbp+24]       # buf  = string pointer
    mov rdx, [rbp+16]       # len  = length argument
    syscall
    mov rax, 0
    mov rsp, rbp
    pop rbp
    ret
fibo:
    push rbp
    mov rbp, rsp
    sub rsp, 0
    mov eax, 0
    push [rbp+16]           # i
    push 0
    pop rbx
    pop rax
    cmp rax, rbx            # i == 0 ?
    sete al
    movzx rax, al
    push rax
    pop rax
    cmp rax, 0
    je label_0
    push 1
    pop rax
    jmp fibo_done           # base case: return 1
    jmp label_1
label_0:
label_1:
    push [rbp+16]           # i
    push 1
    pop rbx
    pop rax
    cmp rax, rbx            # i == 1 ?
    sete al
    movzx rax, al
    push rax
    pop rax
    cmp rax, 0
    je label_2
    push 1
    pop rax
    jmp fibo_done           # base case: return 1
    jmp label_3
label_2:
label_3:
    sub rsp, 8              # 16-byte alignment padding
    push [rbp+16]
    push 1
    pop rbx
    pop rax
    sub rax, rbx            # i - 1
    push rax
    call fibo               # fibo(i - 1)
    add rsp, 16             # clean up arg + padding
    push rax               # keep result live on the stack
    push [rbp+16]
    push 2
    pop rbx
    pop rax
    sub rax, rbx            # i - 2
    push rax
    call fibo               # fibo(i - 2)
    add rsp, 8              # clean up arg
    push rax
    pop rbx
    pop rax
    add rax, rbx            # fibo(i-1) + fibo(i-2)
    push rax
    pop rax
    jmp fibo_done
fibo_done:
    mov rsp, rbp
    pop rbp
    ret
main:
    push rbp
    mov rbp, rsp
    sub rsp, 0
    mov eax, 0
    push 10
    call fibo               # fibo(10)
    add rsp, 8
    push rax
    pop rax
    mov rdi, rax            # exit code = return value
    mov rax, 60             # exit syscall
    syscall
main_done:
    mov rsp, rbp
    pop rbp
    ret
.section .note.GNU-stack,"",@progbits
```

---

## How it works

The compiler is a hand-written pipeline with no external codegen backend:

**Lexer → Parser → Scope check → Type check → Code generation → x86-64 assembly**

Each pass is a visitor over the AST. Scope checking and type checking must pass before any code is generated; code generation lowers the AST directly to Intel-syntax assembly.

## What this demonstrates

- **Recursion** with independent, correctly-unwound stack frames
- **Lexical scoping** with correct variable shadowing across nested blocks
- **16-byte stack alignment** maintained at every call site, per the System V AMD64 ABI
- **A hand-written calling convention** — stack-passed arguments with caller-side cleanup
- **Freestanding output** — no libc and no runtime; `print` is a direct `write` syscall and exit is a direct `exit` syscall
- **Static typing** across `i32`, `bool`, and `string`, with type errors rejected before codegen
- **Control flow** — `if` / `else if` / `else` and the full set of comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`)
- **Direct x86-64 Linux code generation** — no LLVM, no assembler beyond `gas`

## Building and running

```sh
cmake -B build
cmake --build build
./build/Compiler file.sr      # emits output.s
```

Assemble and link the generated freestanding binary, then run it and inspect the exit code:

```sh
as output.s -o output.o
ld output.o -o program        # static, no libc
./program
echo $?                       # exit status = program's return value
```

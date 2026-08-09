# Example

A few simple example of the language:

```rust
hi(): i32 => {
    ret 5 + 10;
}

main(): i32 => {
    let i: i32 = print("hello");
    ret hi();
}
```

### Output

```
hello
```

The program exits with status code:

```
15
```


```rust
add(i: i32, j: i32): i32 => ret i + j;

main(): i32 => {
    let message: string = "hey";
    print(message, add(1, 1));   
    ret add(5, 10);
}
```

### Output

```
he
```

The program exits with status code:

```
15
```


```rust
add(i: i32, j: i32): i32 => ret i + j;

main(): i32 => {
    let message: string = "hey\n";
    print(message, 4);   
    {
        let message: string = "hello\n";
        print(message, 6);
    }
    {
        print(message, 4);
        message = "hi\n";
        print(message, 3);
    }
    print(message, 4);
    ret add(1, 2);
}

```

### Output

```
hey
hello
hey
hi
hi
```

The program exits with status code:

```
3
```



```
---

# Generated x86-64 Assembly (Linux)

The compiler currently emits Intel syntax assembly compatible with `gas`.

```asm
.intel_syntax noprefix

.section .data
string_4:
    .ascii "hello"
len_4 = . - string_4

.section .text
.globl main

print:
    push rbp
    mov rbp, rsp

    mov rax, 1          # sys_write
    mov rdi, 1          # stdout
    mov rsi, [rbp+24]   # string pointer
    mov rdx, [rbp+16]   # string length
    syscall

    mov rax, 0

    mov rsp, rbp
    pop rbp
    ret

hi:
    push rbp
    mov rbp, rsp

    sub rsp, 0
    mov eax, 0

    push 5
    push 10

    pop rbx
    pop rax
    add rax, rbx

    push rax
    pop rax

    jmp hi_done

hi_done:
    mov rsp, rbp
    pop rbp
    ret

main:
    push rbp
    mov rbp, rsp

    sub rsp, 16
    mov eax, 0

    lea rax, [rip + string_4]
    push rax

    mov rax, len_4
    push rax

    call print

    push rax
    pop rax

    mov [rbp-4], eax

    call hi

    push rax
    pop rax

    mov rdi, rax
    mov rax, 60         # sys_exit
    syscall

main_done:
    mov rsp, rbp
    pop rbp
    ret

.section .note.GNU-stack,"",@progbits
```

---

## What this demonstrates

- ✅ Functions with return values
- ✅ Static typing (`i32`)
- ✅ Variable declarations (with shadowing)
- ✅ Function calls
- ✅ String literals
- ✅ Built-in `print()` function
- ✅ Arithmetic expressions
- ✅ Direct x86-64 Linux code generation (no LLVM)

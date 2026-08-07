.intel_syntax noprefix

.section .data
hello: .ascii "hello world!\n"
len = . - hello

.section .text
.globl main
push 1
push 2
hey:
    push rbp
    mov rbp, rsp
sub rsp, 0
    mov eax, 0
    push [rbp+24]
    push [rbp+16]
    pop rbx
    pop rax
    add rax, rbx
    push rax
    pop rax
    jmp hey_done
hey_done:
    mov rsp, rbp
    pop rbp
    ret
main:
    push rbp
    mov rbp, rsp
sub rsp, 16
    mov eax, 0
    push 1
    push 7
    call hey
    push rax
    pop rax
    mov [rbp-4], eax
    push [rbp-4]
    mov rax, 1
    mov rdi, 1
    lea rsi, [rip + hello]
    mov rdx, len
    syscall
    pop rax
    mov rdi, rax
    mov rax, 60
    syscall
main_done:
    mov rsp, rbp
    pop rbp
    ret
.section .note.GNU-stack,"",@progbits

.intel_syntax noprefix
.section .data
string_2:
.ascii "hey"
len_2 = . - string_2
.section .text
.globl _start
_start:
    push rbp
    mov rbp, rsp
sub rsp, 16
    mov rax, 0
    lea rax, [rip + string_2]
    push rax
    pop rax
    mov [rbp-4], rax
    push [rbp-4]
    pop rax
    push rax
    pop rax
    mov [rbp-12], rax
    push 16
    call _malloc
    add rsp, 8
    push rax
    pop rax
    movzx rax, al
    push rax
    pop rax
    mov rdi, rax
    mov rax, 60
    syscall
start_done:
    mov rsp, rbp
    pop rbp
    ret
.section .note.GNU-stack,"",@progbits

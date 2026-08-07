.intel_syntax noprefix
.section .data
string_3:
.ascii "hello"
len_3 = . - string_3
.section .text
.globl main
print:
    push rbp
    mov rbp, rsp
    mov rax, 1
    mov rdi, 1
    mov rsi, [rbp+24]
    mov rdx, [rbp+16]
    syscall
    mov rax, 0
    mov rsp, rbp
    pop rbp
    ret
main:
    push rbp
    mov rbp, rsp
sub rsp, 16
    mov eax, 0
    lea rax, [rip + string_3]
    push rax
    mov rax, len_3
    push rax
    call print
    push rax
    pop rax
    mov [rbp-4], eax
    push 20
    pop rax
    mov rdi, rax
    mov rax, 60
    syscall
main_done:
    mov rsp, rbp
    pop rbp
    ret
.section .note.GNU-stack,"",@progbits

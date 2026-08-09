.intel_syntax noprefix
.section .data
string_5:
.ascii "hey"
len_5 = . - string_5
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
add:
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
    jmp add_done
add_done:
    mov rsp, rbp
    pop rbp
    ret
main:
    push rbp
    mov rbp, rsp
sub rsp, 16
    mov eax, 0
    lea rax, [rip + string_5]
    push rax
    pop rax
    mov [rbp-4], rax
    push [rbp-4]
    push 1
    push 1
    call add
    add rsp, 16
    push rax
    call print
    add rsp, 16
    push rax
    pop rax
    push 5
    push 10
    call add
    add rsp, 16
    push rax
    pop rax
    mov rdi, rax
    mov rax, 60
    syscall
main_done:
    mov rsp, rbp
    pop rbp
    ret
.section .note.GNU-stack,"",@progbits

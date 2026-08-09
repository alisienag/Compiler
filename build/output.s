.intel_syntax noprefix
.section .data
string_5:
.ascii "hey\n"
len_5 = . - string_5
string_7:
.ascii "hello\n"
len_7 = . - string_7
string_8:
.ascii "hi\n"
len_8 = . - string_8
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
    push 4
    call print
    add rsp, 16
    push rax
    pop rax
    lea rax, [rip + string_7]
    push rax
    pop rax
    mov [rbp-12], rax
    push [rbp-12]
    push 6
    call print
    add rsp, 16
    push rax
    pop rax
    push [rbp-4]
    push 4
    call print
    add rsp, 16
    push rax
    pop rax
    lea rax, [rip + string_8]
    push rax
    pop rax
    mov [rbp-4], rax
    push [rbp-4]
    push 3
    call print
    add rsp, 16
    push rax
    pop rax
    push [rbp-4]
    push 4
    call print
    add rsp, 16
    push rax
    pop rax
    push 1
    push 2
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

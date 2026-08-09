.intel_syntax noprefix
.section .data
string_6:
.ascii "go\n"
len_6 = . - string_6
.section .text
.globl _start
_print:
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
_add:
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
_start:
    push rbp
    mov rbp, rsp
sub rsp, 16
    mov eax, 0
    sub rsp, 8
    push 1
    push 1
    call _add
    add rsp, 24
    push rax
    push 2
    pop rbx
    pop rax
    cmp rax, rbx
    sete al
    movzx rax, al
    push rax
    pop rax
    mov [rbp-4], rax
    push [rbp-4]
    pop rax
    cmp rax, 0
    je label_0
    sub rsp, 8
    lea rax, [rip + string_6]
    push rax
    push 3
    call _print
    add rsp, 24
    push rax
    pop rax
    jmp label_1
label_0:
label_1:
    sub rsp, 8
    push 5
    push 10
    call _add
    add rsp, 24
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

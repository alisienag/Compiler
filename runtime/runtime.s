.intel_syntax noprefix
.globl _print
.globl get_memory
.globl _malloc
.globl _free
_malloc:
    push rbp
    mov rbp, rsp
    mov rdi, [rbp+16]
    call malloc
    mov rsp, rbp
    pop rbp
    ret
_free:
    push rbp
    mov rbp, rsp
    mov rdi, [rbp+16]
    call free
    mov rsp, rbp
    pop rbp
    ret
get_memory:
    push rbp
    mov rbp, rsp
    mov rax, 9
    mov rsi, rdi
    mov rdi, 0
    mov rdx, 3
    mov r10, 0x22
    mov r8, -1
    mov r9, 0
    syscall
    mov rsp, rbp
    pop rbp
    ret

_print:
    push rbp
    mov rbp, rsp
    mov rax, 1
    mov rdi, 1
    mov rsi, [rbp+16]
    mov rdx, [rbp+24]
    syscall
    mov rax, 0
    mov rsp, rbp
    pop rbp
    ret

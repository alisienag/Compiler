	.file	"runtime.c"
	.intel_syntax noprefix
	.text
	.p2align 4
	.globl	"malloc"
	.type	"malloc", @function
"malloc":
.LFB1:
	.cfi_startproc
	mov	rax, QWORD PTR "free_list"[rip]
	lea	rsi, [rdi+15]
	and	rsi, -16
	test	rax, rax
	je	.L2
	mov	edx, OFFSET FLAT:"free_list"
	jmp	.L5
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L3:
	test	rax, rax
	je	.L2
.L5:
	mov	rcx, QWORD PTR [rax]
	mov	rdi, rdx
	lea	rdx, [rax+8]
	mov	rax, QWORD PTR [rax+8]
	cmp	rcx, rsi
	jb	.L3
	mov	QWORD PTR [rdi], rax
	mov	rax, rdx
	ret
	.p2align 4,,10
	.p2align 3
.L2:
	mov	rdx, QWORD PTR "heap_ptr"[rip]
	lea	rcx, [rsi+8]
	lea	rax, [rdx+rcx]
	cmp	QWORD PTR "heap_end"[rip], rax
	jb	.L17
	mov	QWORD PTR [rdx], rsi
	add	rdx, 8
	mov	QWORD PTR "heap_ptr"[rip], rax
	mov	rax, rdx
	ret
	.p2align 4,,10
	.p2align 3
.L17:
	mov	edi, 65536
	sub	rsp, 40
	.cfi_def_cfa_offset 48
	cmp	rcx, rdi
	mov	QWORD PTR [rsp+24], rsi
	cmovnb	rdi, rcx
	mov	QWORD PTR [rsp+16], rcx
	mov	QWORD PTR [rsp+8], rdi
	call	"get_memory"
	mov	rdi, QWORD PTR [rsp+8]
	mov	rcx, QWORD PTR [rsp+16]
	mov	rsi, QWORD PTR [rsp+24]
	mov	rdx, rax
	add	rdi, rax
	add	rdx, 8
	lea	rax, [rax+rcx]
	mov	QWORD PTR [rdx-8], rsi
	mov	QWORD PTR "heap_ptr"[rip], rax
	mov	rax, rdx
	mov	QWORD PTR "heap_end"[rip], rdi
	add	rsp, 40
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1:
	.size	"malloc", .-"malloc"
	.p2align 4
	.globl	"free"
	.type	"free", @function
"free":
.LFB2:
	.cfi_startproc
	test	rdi, rdi
	je	.L18
	mov	rax, QWORD PTR "free_list"[rip]
	sub	rdi, 8
	mov	QWORD PTR [rdi+8], rax
	mov	QWORD PTR "free_list"[rip], rdi
.L18:
	ret
	.cfi_endproc
.LFE2:
	.size	"free", .-"free"
	.local	"free_list"
	.comm	"free_list",8,8
	.local	"heap_end"
	.comm	"heap_end",8,8
	.local	"heap_ptr"
	.comm	"heap_ptr",8,8
	.ident	"GCC: (GNU) 16.2.1 20260810"
	.section	.note.GNU-stack,"",@progbits

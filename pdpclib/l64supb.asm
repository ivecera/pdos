# l64supb.asm - support code for C programs for x64 Linux
# For code compiled with the cc64 compiler that follows the
# Microsoft Win64 calling convention
#
# This program written by Paul Edwards
# Released to the public domain


.code64



# This is the new entry point. We simply capture
# the current status of the stack and pass it on
# to C code (linstart.c, not start.c) figure out
# the rest. However, it seems that the MSABI
# requires some stack to be available so that the
# 4 register parameters can be saved if required.
# Which seems to be what cc64 does unconditionally.
# Which means we wipe out the first 4 arguments
# (argv[x]) that were put on the stack. argc survives
# presumably because it is in the spot where the
# return address normally is. Bottom line is we
# need to subtract 48 bytes from the stack before
# things get clobbered.



        .globl ___pdpstart
___pdpstart:
mov %rsp, %rcx
sub $48, %rsp
call _start

# we shouldn't get here, but if we do, loop
# instead of doing something random
loop1:  jmp loop1





.globl ___setj
___setj:
.globl __setj
__setj:
mov $0, %rax
ret

mov 4(%rsp), %rax
push %rbx
mov %rsp, %rbx
mov %rbx, 20(%rax) #esp

mov %rbp, %rbx
mov %rbx, 24(%rax)

mov %rcx, 4(%rax)
mov %rdx, 8(%rax)
mov %rdi, 12(%rax)
mov %esi, 16(%rax)

mov 4(%rsp), %rbx    # return address
mov %rbx, 28(%rax)   # return address

pop %rbx
mov %rbx,0(%rax)
mov $0, %rax

ret



.globl ___longj
___longj:
.globl __longj
__longj:
fred: jmp fred
mov $0, %rax
ret

mov 4(%rsp), %rax
mov 20(%rax), %rbp
mov %rbp, %rsp

pop %rbx            # position of old ebx
pop %rbx            # position of old return address

mov 28(%rax), %rbx  # return address
push %rbx

mov 24(%rax), %rbx
mov %rbx, %rbp

mov 0(%rax), %rbx
mov 4(%rax), %rcx
mov 8(%rax), %rdx
mov 12(%rax), %rdi
mov 16(%rax), %esi

mov 60(%rax), %rax    # return value

ret



.globl __seek
__seek:
.globl ___seek
___seek:

/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif */

# function code 8 = lseek
movq $8, %rax

/* .if STACKPARM
# handle
movq 16(%rbp), %rdi
# offset
movq 24(%rbp), %rsi
# whence
movq 32(%rbp), %rdx
.endif */

syscall

/* .if STACKPARM
pop %rdx
pop %rsi
pop %rdi
pop %rbp
.endif */

ret



.globl ___rename
___rename:
.globl __rename
__rename:

/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
.endif */

# function code 82 = rename
movq $82, %rax

/* .if STACKPARM
# old file
movq 16(%rbp), %rdi
# new file
movq 24(%rbp), %rsi
.endif */

syscall

/* .if STACKPARM
pop %rsi
pop %rdi
pop %rbp
.endif */

ret


.globl ___remove
___remove:
.globl __remove
__remove:

/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
.endif */

# function code 87 = unlink
movq $87, %rax

/* .if STACKPARM
# filename
movq 16(%rbp), %rdi
.endif */

syscall

/* .if STACKPARM
pop %rdi
pop %rbp
.endif */

ret



.globl ___exita
___exita:
.globl __exita
__exita:
# exit/terminate

/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
movq 16(%rbp), %rdi
.endif */

movq $9, %rdi
movq $60, %rax

syscall

/* .if STACKPARM
pop %rdi
pop %rbp
.endif */

ret


.globl ___time
___time:
.globl __time
__time:

/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
.endif */

# function code 201 = retrieve current time
movq $201, %rax

/* .if STACKPARM
# pointer to time_t
movq 16(%rbp), %rdi
.endif */

syscall

/* .if STACKPARM
pop %rdi
pop %rbp
.endif */

ret



# int ___ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);

.globl ___ioctl
___ioctl:
.globl __ioctl
__ioctl:

push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx

# function code 16 = ioctl
movq $16, %rax

/* .if STACKPARM
# file descriptor
movq 16(%rbp), %rdi
# command
movq 24(%rbp), %rsi
# parameter
movq 32(%rbp), %rdx
.endif */

# handle
xor %rdi,%rdi
mov %ecx, %edi
# cmd
mov %rdx, %rsi
# arg
mov %r8, %rdx

syscall

pop %rdx
pop %rsi
pop %rdi
pop %rbp

ret



.globl ___chdir
___chdir:
.globl __chdir
__chdir:

/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
.endif */

# function code 80 = chdir
movq $80, %rax

/* .if STACKPARM
# filename (directory name)
movq 16(%rbp), %rdi
.endif */

syscall

/* .if STACKPARM
pop %rdi
pop %rbp
.endif */

ret




.globl ___rmdir
___rmdir:
.globl __rmdir
__rmdir:
/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
.endif */

# function code 84 = rmdir
movq $84, %rax

/* .if STACKPARM
# pathname
movq 8(%rbp), %rdi
.endif */

syscall

/* .if STACKPARM
pop %rdi
pop %rbp
.endif */

ret




.globl ___mkdir
___mkdir:
.globl __mkdir
__mkdir:

/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
.endif */

# function code 83 = mkdir
movq $83, %rax

/* .if STACKPARM
# pathname
movq 16(%rbp), %rdi
# mode
movq 24(%rbp), %rsi
.endif */

syscall

/* .if STACKPARM
pop %rsi
pop %rdi
pop %rbp
.endif */

ret




.globl ___getdents
___getdents:
.globl __getdents
__getdents:

/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif */

# function code 78 = getdents
movq $78, %rax

/* .if STACKPARM
# file descriptor
movq 16(%rbp), %rdi
# dirent
movq 24(%rbp), %rsi
# count
movq 32(%rbp), %rdx
.endif */

syscall

/* .if STACKPARM
pop %rdx
pop %rsi
pop %rdi
pop %rbp
.endif */

ret




.globl ___mprotect
___mprotect:
.globl __mprotect
__mprotect:

/* .if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif */

# function code 10 = mprotect
movq $10, %rax

/* .if STACKPARM
# start
movq 16(%rbp), %rdi
# len
movq 24(%rbp), %rsi
# prot
movq 32(%rbp), %rdx
.endif */

syscall

/* .if STACKPARM
pop %rdx
pop %rsi
pop %rdi
pop %rbp
.endif */

ret










.intel_syntax noprefix


.globl __write
__write:

push rdi
push rsi
push rdx

# function code 1 = write
mov rax, 1

# handle
xor rdi, rdi
mov edi, ecx
# data pointer
mov rsi, rdx
xor rdx, rdx
# length
xor rdx, rdx
mov edx, r8d

syscall

pop rdx
pop rsi
pop rdi

ret






.globl __open
__open:

push rdi
push rsi
push rdx

# function code 2 = open
mov rax, 2

# filename
mov rdi,rcx
# flag
xor rsi, rsi
mov esi, edx
# mode
xor rdx, rdx
mov edx, r8d

syscall

pop rdx
pop rsi
pop rdi

ret



.globl __read
__read:

push rdi
push rsi
push rdx

# function code 0 = read
mov rax, 0

# handle
xor rdi, rdi
mov edi, ecx
# data pointer
mov rsi, rdx
# length
# Not sure if this is 32 bits or 64 bits
# cc64 will be using a 32-bit value. And may not
# be zeroing the high 32 bits. Linux may require
# a full clean 64-bit value
xor rdx, rdx
mov edx, r8d
#mov rdx, r8

syscall

pop rdx
pop rsi
pop rdi

ret



.globl __close
__close:

push rdi

# function code 3 = close
mov rax, 3

# handle
xor rdi, rdi
mov edi, ecx

syscall

pop rdi

ret






# These routines were copied (and then modified) from bcclib.asm generated
# by the public domain cc64, and are used for cc64
# (Converted to PDAS .intel_syntax noprefix by guessing.)

.globl m$ufloat_r64u32
m$ufloat_r64u32:
	mov ecx, ecx					# clear top half (already done if value just moved there)
	cvtsi2sd xmm15, rcx
	ret

.globl m$ufloat_r32u32
m$ufloat_r32u32:
	mov ecx, ecx
	cvtsi2ss xmm15, rcx
	ret

.globl m$ufloat_r64u64
m$ufloat_r64u64:
#jjj: jmp jjj
#mov rax, 0x1234567
#mov rax, 1234567
#xorpd xmm15, xmm15
#ret

	cmp ecx, 0
	jl fl1
#number is positive, so can treat like i64
	cvtsi2sd xmm15, rcx
	ret
fl1:						#negative value
	and rcx, QWORD PTR mask63[rip]		#clear top bit (subtract 2**63)
	cvtsi2sd xmm15, rcx
	addsd xmm15, QWORD PTR offset64[rip]	#(add 2**63 back to result)
	ret

.globl m$ufloat_r32u64
m$ufloat_r32u64:
	cmp ecx, 0
	jl fl2
#number is positive, so can treat like i64
	cvtsi2ss xmm15, rcx
	ret
fl2:						#negative value
	and rcx, QWORD PTR mask63[rip]		#clear top bit (subtract 2**63)
	cvtsi2ss xmm15, rcx
	addss xmm15, DWORD PTR offset32[rip]	#(add 2**63 back to result)
	ret

.globl m$pushcallback
m$pushcallback:
	incd __ncallbacks[rip]
        push r12
	mov r12,__ncallbacks[rip]
	shl r12,6					#8x8 bytes is size per entry
	lea r11,callbackstack[rip]
	add r11,r12
        pop r12

	mov [r11],rbx
	mov [r11+8],rsi
	mov [r11+16],rdi
	mov [r11+24],r12
	mov [r11+32],r13
	mov [r11+40],r14
	mov [r11+48],r15
	ret

.globl m$popcallback
m$popcallback:
        mov r12,__ncallbacks[rip]
	shl r12,6					#8x8 bytes is size per entry
	lea r11,callbackstack[rip]
	add r11,r12
	mov rbx,[r11]
	mov rsi,[r11+8]
	mov rdi,[r11+16]
	mov r12,[r11+24]
	mov r13,[r11+32]
	mov r14,[r11+40]
	mov r15,[r11+48]
	decd __ncallbacks[rip]
	ret

.data

.globl _fltused
.p2align 2
_fltused:
.space 4

.section rdata
mask63:
	.quad 0x7fffffffffffffff
offset64:
	.quad 9223372036854775808		# 2**63 as r64
offset32:
	.quad 9223372036854775808		# 2**63 as r32


.bss
callbackstack:
	.space 576			#8-level stack
#	resb 5'120'000

.globl __ncallbacks
__ncallbacks:
	.space 8

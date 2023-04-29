/ support routines
/ written by Paul Edwards
/ released to the public domain

        .globl _int86
        .globl _int86x
        .globl _enable
        .globl _disable
        .globl ___setj
        .globl ___longj
        .globl _inp
        .globl _inpw
        .globl _inpd
        .globl _outp
        .globl _outpw
        .globl _outpd
        .globl _hltintgo
        .globl _hltinthit
        .globl ___switch
        .globl ___brkpoint
        .globl ___brkpoint2

        .text

/ Because of the C calling convention, and the fact that the seg
/ regs are the last parameter, and they're not actually used, the
/ entry point for _int86 can be reused for _int86x
_int86x:
_int86:
        push    %ebp
        mov     %esp, %ebp
        push    %eax
        push    %ebx
        push    %ecx
        push    %edx
        push    %esi
        push    %edi

        mov     12(%ebp), %esi
        mov     0(%esi), %eax
        mov     4(%esi), %ebx
        mov     8(%esi), %ecx
        mov     12(%esi), %edx

        / preserve ebp
        push    %ebp
        / next is actually ebp
        mov     32(%esi), %edi
        / new ebp ready on stack
        push    %edi

        mov     20(%esi), %edi
        mov     16(%esi), %esi
              
        cmpl    $0x10, 8(%ebp)
        jne     not10
        pop     %ebp
        int     $0x10
        jmp     fintry
not10:

        cmpl    $0x13, 8(%ebp)
        jne     not13
        pop     %ebp
        int     $0x13
        jmp     fintry
not13:

        cmpl    $0x14, 8(%ebp)
        jne     not14
        pop     %ebp
        int     $0x14
        jmp     fintry
not14:

        cmpl    $0x15, 8(%ebp)
        jne     not15
        pop     %ebp
        int     $0x15
        jmp     fintry
not15:

        cmpl    $0x16, 8(%ebp)
        jne     not16
        pop     %ebp
        int     $0x16
        jmp     fintry
not16:

        cmpl    $0x1A, 8(%ebp)
        jne     not1A
        pop     %ebp
        int     $0x1A
        jmp     fintry  
not1A:

        cmpl    $0x20, 8(%ebp)
        jne     not20
        pop     %ebp
        int     $0x20
        jmp     fintry
not20:

        cmpl    $0x21, 8(%ebp)
        jne     not21
        pop     %ebp
        int     $0x21
        jmp     fintry
not21:

        cmpl    $0x25, 8(%ebp)
        jne     not25
        pop     %ebp
        int     $0x25
        jmp     fintry
not25:

        cmpl    $0x26, 8(%ebp)
        jne     not26
        pop     %ebp
        int     $0x26
        jmp     fintry
not26:

        cmpl    $0x80, 8(%ebp)
        jne     not80
        pop     %ebp
        int     $0x80
        jmp     fintry
not80:

/ Copied BIOS interrupts.
        cmpl    $0xA0, 8(%ebp)
        jne     notA0
        pop     %ebp
        int     $0xA0
        jmp     fintry
notA0:

        cmpl    $0xA3, 8(%ebp)
        jne     notA3
        pop     %ebp
        int     $0xA3
        jmp     fintry
notA3:

        cmpl    $0xA4, 8(%ebp)
        jne     notA4
        pop     %ebp
        int     $0xA4
        jmp     fintry
notA4:

        cmpl    $0xA5, 8(%ebp)
        jne     notA5
        pop     %ebp
        int     $0xA5
        jmp     fintry
notA5:

        cmpl    $0xA6, 8(%ebp)
        jne     notA6
        pop     %ebp
        int     $0xA6
        jmp     fintry
notA6:

        cmpl    $0xAA, 8(%ebp)
        jne     notAA
        pop     %ebp
        int     $0xAA
        jmp     fintry
notAA:

/ any unknown interrupt still needs to clean up ebp
        pop     %ebp

fintry:
        / new result
        push    %ebp
        push    %esi
        / this is the old value, not new result
        mov     8(%esp), %ebp
        mov     16(%ebp), %esi
        mov     %eax, 0(%esi)
        / actually new ebp
        mov     4(%esp), %eax
        / new ebp
        mov     %eax, 32(%esi)
        mov     %ebx, 4(%esi)
        mov     %ecx, 8(%esi)
        mov     %edx, 12(%esi)
        mov     %edi, 20(%esi)

/ this is actually esi
        pop     %eax
        mov     %eax, 16(%esi)
        mov     $0, %eax
        mov     %eax, 24(%esi)
        jnc     nocarry
        mov     $1, %eax
        mov     %eax, 24(%esi)
nocarry:        
        pushf
        pop     %eax
        mov     %eax, 28(%esi)

        / we already popped esi, but the new ebp hasn't been popped yet
        / we don't actually need that value anymore, so clobber eax instead
        pop     %eax
        / we already have this value loaded, but we need to
        / get the stack back to previous state, so pop the same value
        pop     %ebp

        pop     %edi
        pop     %esi
        pop     %edx
        pop     %ecx
        pop     %ebx
        pop     %eax
        pop     %ebp
        ret

_enable:
        sti
        ret

_disable:
        cli
        ret

/ read a character from a port
_inp:
        push    %ebp
        mov     %esp, %ebp
        push    %edx
        mov     8(%ebp), %edx
        mov     $0, %eax
        inb     %dx, %al
        pop     %edx
        pop     %ebp
        ret

/ read a word from a port
_inpw:
        push    %ebp
        mov     %esp, %ebp
        push    %edx
        mov     8(%ebp), %edx
        mov     $0, %eax
        inw     %dx, %ax
        pop     %edx
        pop     %ebp
        ret

/ read a dword from a port
_inpd:
        push    %ebp
        mov     %esp, %ebp
        push    %edx
        mov     8(%ebp), %edx
        mov     $0, %eax
        inl     %dx, %eax
        pop     %edx
        pop     %ebp
        ret

/ write a character to a port
_outp:
        push    %ebp
        mov     %esp, %ebp
        push    %edx
        mov     8(%ebp), %edx
        mov     12(%ebp), %eax
        outb    %al, %dx
        pop     %edx
        pop     %ebp
        ret

/ write a word to a port
_outpw:
        push    %ebp
        mov     %esp, %ebp
        push    %edx
        mov     8(%ebp), %edx
        mov     12(%ebp), %eax
        outw    %ax, %dx
        pop     %edx
        pop     %ebp
        ret

/ write a dword to a port
_outpd:
        push    %ebp
        mov     %esp, %ebp
        push    %edx
        mov     8(%ebp), %edx
        mov     12(%ebp), %eax
        outl    %eax, %dx
        pop     %edx
        pop     %ebp
        ret


/ enable interrupts and then halt until interrupt hit
_hltintgo:
hloop:
/ I believe hlt will be interrupted by other interrupts, like
/ the timer interrupt, so we need to do it in a loop
        sti
        hlt
        cli
        jmp     hloop
_hltinthit:
/ remove return address, segment and flags from the stack as we
/ do not intend to return to the jmp following the hlt instruction
/ that was likely interrupted
        add     $12, %esp
/ note that interrupts will be disabled again (I think) by virtue
/ of the fact that an interrupt occurred. The caller would have
/ disabled interrupts already, so we are returning to the same
/ disabled state.
        ret


# Basically copied from linsupa.asm

.globl ___setj
___setj:
mov 4(%esp), %eax
push %ebx
mov %esp, %ebx
mov %ebx, 20(%eax) #esp

mov %ebp, %ebx
mov %ebx, 24(%eax)

mov %ecx, 4(%eax)
mov %edx, 8(%eax)
mov %edi, 12(%eax)
mov %esi, 16(%eax)

mov 4(%esp), %ebx    # return address
mov %ebx, 28(%eax)   # return address

pop %ebx
mov %ebx,0(%eax)
mov $0, %eax

ret



.globl ___longj
___longj:
mov 4(%esp), %eax
mov 20(%eax), %ebp
mov %ebp, %esp

pop %ebx            # position of old ebx
pop %ebx            # position of old return address

mov 28(%eax), %ebx  # return address
push %ebx

mov 24(%eax), %ebx
mov %ebx, %ebp

mov 0(%eax), %ebx
mov 4(%eax), %ecx
mov 8(%eax), %edx
mov 12(%eax), %edi
mov 16(%eax), %esi

mov 60(%eax), %eax    # return value

ret


# From and for SubC
# internal switch(expr) routine
# %esi = switch table, %eax = expr

___switch:	pushl	%esi
	movl	%edx,%esi
	movl	%eax,%ebx
	cld
	lodsl
	movl	%eax,%ecx
next:	lodsl
	movl	%eax,%edx
	lodsl
	cmpl	%edx,%ebx
	jnz	no
	popl	%esi
	jmp	*%eax
no:	loop	next
	lodsl
	popl	%esi
	jmp	*%eax


.globl ___brkpoint
___brkpoint:
        int     $0x3
        ret


.globl ___brkpoint2
___brkpoint2:
        int     $0x3
        ret

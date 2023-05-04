; winsupa.asm - support code for C programs for Win32
;
; This program written by Paul Edwards
; Released to the public domain

.386
.model flat, c

.code

; language C doesn't seem to be working for wasm

ifdef WATCOM
public ___setj
public ___longj
endif

public __setj
public __longj
public __chkstk_ms


ifdef WATCOM
___setj:
endif

__setj proc
        mov eax, [esp+4]
        push ebx
        mov ebx, esp
        mov [eax+20], ebx    ; esp

        mov ebx, ebp
        mov [eax+24], ebx    ; ebp

        mov [eax+4], ecx
        mov [eax+8], edx
        mov [eax+12], edi
        mov [eax+16], esi

        mov ebx, [esp+4]   ; return address
        mov [eax+28], ebx    ; return address

        pop ebx
        mov [eax+0], ebx
        mov eax, 0

        ret
__setj endp

ifdef WATCOM
___longj:
endif

__longj proc
        mov eax, [esp+4]
        mov ebp, [eax+20]
        mov esp, ebp

        pop ebx                ; position of old ebx
        pop ebx                ; position of old return address

        mov ebx, [eax+28]    ; return address
        push ebx

        mov ebx, [eax+24]
        mov ebp, ebx

        mov ebx, [eax+0]
        mov ecx, [eax+4]
        mov edx, [eax+8]
        mov edi, [eax+12]
        mov esi, [eax+16]

        mov eax, [eax+32]    ; return value

        ret
__longj endp

; For compiling with GCC 4 we don't want to
; have to link the GCC library in
__chkstk_ms proc
        ret
__chkstk_ms endp

end

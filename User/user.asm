BITS 32
ORG 0x40000000

_start:
    mov eax, 0          ; syscall: print
    mov ebx, msg
    int 0x80

hang:
    jmp hang

msg db "Hello from user space!", 0

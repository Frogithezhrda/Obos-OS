[BITS 32]
org 0x00100000
mov word [0xB8000], 0x0F4C  ; 'K'
hlt
jmp $

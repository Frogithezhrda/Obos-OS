[BITS 32]
org 0x100000

mov byte [0xB8000], 'K'      ; Character
mov byte [0xB8001], 0x0F     ; Attribute (white on black)

; IT WILL SHOW THE X IN THE TOP LEFT OF THE SCREEN BECAUSE WE ARE IN VGA
.hang:
    hlt
    jmp .hang
; times 512 -($ - $$) db 0
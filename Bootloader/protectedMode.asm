[BITS 32]
ProtectedModeEntry:
    mov ax, DATA_OFFSET
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax          
    mov gs, ax
    mov ebp, 0x9C00
    mov esp, ebp
    
    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    mov byte [0xB8000], 'X'      ; Character
    mov byte [0xB8001], 0x0F     ; Attribute (white on black)
    ; IT WILL SHOW THE X IN THE TOP LEFT OF THE SCREEN BECAUSE WE ARE IN VGA
Halt:
    hlt
    jmp Halt
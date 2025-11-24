[BITS 16]

; i used this as a reference https://wiki.osdev.org/GDT_Tutorial

LoadGDT:
NullGDT:
    dq 0x0000000000000000

CodeGDT:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A      
    db 0xCF      
    db 0x00

DataGDT:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92      
    db 0xCF      
    db 0x00

EndGDT:

GDTDescriptor:
    dw EndGDT - LoadGDT - 1
    dd LoadGDT
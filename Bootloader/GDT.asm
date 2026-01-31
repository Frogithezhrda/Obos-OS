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


UserCodeGDT:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0xFA                         ; Present=1, DPL=3, Code segment, Executable=1, Readable=1
    db 0xCF
    db 0x00

UserDataGDT:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0xF2                         ; Present=1, DPL=3, Data segment, Writable=1
    db 0xCF
    db 0x00

TSSDescriptor:
    dw 0x0067                       ; Limit
    dw 0x0000                       ; Base low
    db 0x00                         ; Base mid
    db 0x89                         ; Access byte: Present=1, DPL=0, Type=32-bit TSS
    db 0x40                         ; Flags: byte granularity, 32-bit
    db 0x00                         ; Base high
EndGDT:

GDTDescriptor:
    dw EndGDT - LoadGDT - 1
    dd LoadGDT


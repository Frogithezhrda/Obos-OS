

; i used this as a reference https://wiki.osdev.org/GDT_Tutorial
LoadGDT:
    
    NullGDT:
        db 0x0000
        db 0x0000

    CodeGDT:
        dw 0xFFFF   ; Limit
        dw 0x0000   ; Base
        db 0x00     ; Base
        db 0x9A     ; Acess Byte
        db 0xC      ; Flags
        db 0x00     ; Base

    DataGDT:
        dw 0xFFFF   ; Limit
        dw 0x0000   ; Base
        db 0x00     ; Base
        db 0xF2     ; Acess Byte
        db 0xC      ; Flags
        db 0x00     ; Base

    EndGDT:

GDTDescriptor:
    dw EndGDT - LoadGDT - 1
    dd LoadGDT



        


        
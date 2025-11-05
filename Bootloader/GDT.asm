

; i used this as a reference https://wiki.osdev.org/GDT_Tutorial
GDTTable:
    
    GDTNull:
        db 0x0000
        db 0x0000


    GDTCode:
        dw 0xFFFF   ; Limit
        dw 0x0000   ; Base
        db 0x00     ; Base
        db 0x9A     ; Acess Byte
        db 0xC      ; Flags
        db 0x00     ; Base


    GDTData:
        dw 0xFFFF   ; Limit
        dw 0x0000   ; Base
        db 0x00     ; Base
        db 0xF2     ; Acess Byte
        db 0xC      ; Flags
        db 0x00     ; Base

    GDTEnd:



GDTDescriptor:
    dw GDTEnd - GDTTable - 1
    dd GDTTable



        


        
[BITS 16]   ; Telling It 16 Bits

org 0x7C00  ; Telling The Bootloader That It Starts at 0x7C00

; Data Segment
bootDrive db 0

; Code Segment(For More Convienece)
jmp Start

Start:
    cli             ; Remove Any External Hardware Interrupts
    xor ax, ax      ; Setting All The Segments To 0x0000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFF  ; Init The Stack Pointer to the top

    sti             ; Enable Back Interrupts    
    
    jmp LoadGDTTable; Jump To Loading The GDT Table
    

LoadGDTTable:
    cli
    lgdt[GDTTable]
    mov eax, cr0
    or al, 0x01
    mov cr0, eax
    jmp 0x0008:ProtectedMode ; Far Jump To Protected Mode



%include "Bootloader/GDT.asm"
%include "Bootloader/protectedMode.asm"

times 510 - ($ - $$) db 0   ; Getting The Rest Of The 512 Bytes(Padding)
dw 0xAA55           ; Signature
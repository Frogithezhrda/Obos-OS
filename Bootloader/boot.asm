[BITS 16]   ; Telling its 16 Bits

org 0x7C00  ; Telling the Bootloader that it starts at 0x7C00

; Data Segment
bootDrive db 0
CODE_OFFSET equ 0x0008

; Code Segment(for more convienece)
jmp Start

Start:
    cli             ; Remove any external hardware interrupts
    
    xor ax, ax      ; Setting all the segments to 0x0000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000  ; Init the stack pointer to the top

    sti             ; Enable back interrupts    

    cld
    
LoadGDTl:
    cli             ; Remove any external hardware interrupts

    lgdt[LoadGDT]   ; Load GDT

SwitchToProtectedMode:
    mov eax, cr0    ; Load cr0 curr config (cr0 is register who controls basic CPU operation)
    or al, 0x01     ; Turn Protected Mode on
    mov cr0, eax    ; Update cr0 for the CPU to run in Protected Mode

    jmp CODE_OFFSET:ProtectedMode ; Far jump to protected mode


%include "Bootloader/protectedMode.asm"
[BITS 16]
%include "Bootloader/GDT.asm"

times 510 - ($ - $$) db 0   ; Fill the rest of the 512 Bytes(padding)
dw 0xAA55                   ; Signature



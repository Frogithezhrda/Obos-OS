[BITS 16]
org 0x7C00

bootDrive db 0
CODE_OFFSET equ 0x08
DATA_OFFSET equ 0x10

jmp Start

Start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    sti
    cld

LoadGDTl:
    cli
    lgdt [GDTDescriptor]
    
SwitchToProtectedMode:
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_OFFSET:ProtectedModeEntry

%include "Bootloader/GDT.asm"
%include "Bootloader/protectedMode.asm"


times 510 - ($ - $$) db 0
dw 0xAA55
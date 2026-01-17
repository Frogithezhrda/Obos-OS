[BITS 16]
org 0x7C00

CODE_OFFSET equ 0x08
DATA_OFFSET equ 0x10

KERNEL_TEMP_ADDR equ 0x1000 ; temporary address for the kernel
KERNEL_START_ADDR equ 0x00100000
DAP_ADDRESS        equ 0x0600
KERNEL_SECTORS     equ 20 ; change according to the current Kernel size(sectors)
SEGMENT_SIZE equ 512
mmap_ent equ 0x8000

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
    
EnableA20:
    in al, 0x92
    or al, 2
    out 0x92, al
    call E820


LoadGDTl:
    cli
    lgdt [GDTDescriptor]
    jmp SwitchToProtectedMode


SwitchToProtectedMode:
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_OFFSET:ProtectedModeEntry

%include "Bootloader/e820.asm"
%include "Bootloader/GDT.asm"
%include "Bootloader/protectedMode.asm"

times 510 - ($ - $$) db 0
dw 0xAA55

[BITS 16]
org 0x7C00

bootDrive db 0

CODE_OFFSET equ 0x08
DATA_OFFSET equ 0x10

KERNEL_LOAD_SEG equ 0x1000
KERNEL_START_ADDR equ 0x100000

DAP_ADDRESS equ 0x0600

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
    

CreateDAP:
    mov byte [DAP_ADDRESS], 16       ;DAP sizeA
    mov byte [DAP_ADDRESS + 1], 0    ;Reserved

    mov word [DAP_ADDRESS + 2], 1    ;Number od secttors to read

    mov word [DAP_ADDRESS + 4], 0        ;Dest offset
    mov word [DAP_ADDRESS + 6], 0x1000   ;Dest segment

    mov dword [DAP_ADDRESS + 8], 1    ;LBA low
    mov dword [DAP_ADDRESS + 12], 0   ;LBA high
    

LoadKernelWithINT13:
    mov ah, 0x42 ;Extended Read

    mov dl, 0x80 ;HardDisk0
    mov si, DAP_ADDRESS
    mov ax, 0x0000
    mov es, ax

    int 0x13

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
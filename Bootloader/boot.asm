[BITS 16]
org 0x7C00

CODE_OFFSET equ 0x08
DATA_OFFSET equ 0x10

KERNEL_TEMP_ADDR equ 0x1000 ; temporary address for the kernel
KERNEL_START_ADDR equ 0x00100000
DAP_ADDRESS        equ 0x0600
KERNEL_SECTORS     equ 20 ; change according to the current Kernel size(sectors)
SEGMENT_SIZE equ 512
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
    mov byte [DAP_ADDRESS + 0], 16
    mov byte [DAP_ADDRESS + 1], 0
    mov word [DAP_ADDRESS + 2], KERNEL_SECTORS
    mov word [DAP_ADDRESS + 4], KERNEL_TEMP_ADDR  ; temporary address
    mov word [DAP_ADDRESS + 6], 0x0000            ; segment
    mov dword [DAP_ADDRESS + 8], 1
    mov dword [DAP_ADDRESS + 12], 0
    ; Enable A20
EnableA20:
    in al, 0x92
    or al, 2
    out 0x92, al

LoadKernelWithINT13:
    mov ax, 0x0000
    mov es, ax
    mov si, DAP_ADDRESS
    mov ah, 0x42 ;Extended Read
    mov dl, 0x80 ;HardDisk0
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

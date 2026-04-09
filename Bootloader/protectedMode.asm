[BITS 32]

ProtectedModeEntry:
    mov ax, DATA_OFFSET
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax          
    mov gs, ax
    mov ebp, 0x9C00
    mov esp, ebp
    
    mov bl, 254
    mov edi, KERNEL_START_ADDR
    mov ecx, 1
    call ataReadSectors
    jmp KERNEL_START_ADDR

LoadError:
    cli
    hlt
    jmp LoadError

Halt:
    hlt
    jmp Halt

%include "Bootloader/ata.asm"
[BITS 32]

ProtectedModeEntry:
    mov ax, DATA_OFFSET
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x9C00

    mov edi, KERNEL_START_ADDR
    push 1
    pop ecx                 ; LBA = 1 (3 bytes instead of 5)
    mov bl, 127             ; 127 sectors per call
    push 4
    pop esi                 ; 4 chunks = 508 sectors, same as before
.load:
    call ataReadSectors
    add edi, 127*512
    add ecx, 127
    dec esi
    jnz .load

    jmp KERNEL_START_ADDR

%include "Bootloader/ata.asm"
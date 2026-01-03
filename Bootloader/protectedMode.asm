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
    


    mov esi, KERNEL_TEMP_ADDR              ; source (temp location)
    mov edi, KERNEL_START_ADDR   ; destination (1MB)
    mov eax, KERNEL_SECTORS
    mov ebx, SEGMENT_SIZE
    mul ebx
    mov ecx, eax         ; size in bytes (1 sector)
    rep movsb
    jmp KERNEL_START_ADDR

Halt:
    hlt
    jmp Halt

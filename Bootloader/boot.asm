[BITS 16]
org 0x7C00

CODE_OFFSET        equ 0x08
DATA_OFFSET        equ 0x10
KERNEL_TEMP_ADDR   equ 0x1000
KERNEL_START_ADDR  equ 0x00100000
DAP_ADDRESS        equ 0x0600
KERNEL_SECTORS     equ 512
SEGMENT_SIZE       equ 512
mmap_ent           equ 0x8000
VBE_MODE           equ 0x0118
VBEInfoBuf         equ 0x7E00
ModeInfoBuf        equ 0x8200
FB_BASE_STORE      equ 0x9000

jmp Start

Start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xA000
    sti
    cld

EnableA20:
    in al, 0x92
    or al, 2
    out 0x92, al

    call E820
    call QueryVBE

LoadGDTl:
    cli
    lgdt [GDTDescriptor]
    jmp SwitchToProtectedMode

QueryVBE:
    mov ax, VBEInfoBuf
    shr ax, 4
    mov es, ax
    mov di, VBEInfoBuf
    and di, 0x000F
    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x004F
    jne VBEFail

    mov ax, ModeInfoBuf
    shr ax, 4
    mov es, ax
    mov di, ModeInfoBuf
    and di, 0x000F
    mov ax, 0x4F01
    mov cx, VBE_MODE
    int 0x10
    cmp ax, 0x004F
    jne VBEFail

    mov bx, di
    add bx, 0x28
    mov eax, [es:bx]
    mov [FB_BASE_STORE], eax

    call SetVBEMode
    ret

VBEFail:
    cli
    hlt

SetVBEMode:
    mov ax, 0x4F02
    mov bx, 0x4118
    int 0x10
    cmp ax, 0x004F
    jne VBEFail
    ret

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
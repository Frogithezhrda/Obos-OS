[BITS 32]

; Data Segment
DATA_OFFSET equ 0x0010
CODE_OFFSET equ 0x0008


; Code Segment
ProtectedMode:
    mov ax, DATA_OFFSET ; Data Offset
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov gs, ax
    mov ebp, 0x9C00     ; Starting The Stack At 9C00
    mov esp, ebp        ; Stack Pointer To That Address

    ; Enable The A20 If Activated It Doesn't Matter
	mov ax, 2401h               
	int 15h
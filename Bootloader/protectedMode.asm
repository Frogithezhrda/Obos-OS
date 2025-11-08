[BITS 32]

; Data Segment
DATA_OFFSET equ 0x0010

; Code Segment
ProtectedMode:
    mov ax, DATA_OFFSET ; Data Offset 0x0011|FFFF
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

LoadKernel:
    ; Printing “Booting…” to the screen
    mov ah, 0x0E     ; Function: teletype output
    mov al, 0x42     ; 'B'
    int 0x10
    mov al, 0x6F     ; 'o'
    int 0x10
    int 0x10
    mov al, 0x74     ; 't'
    int 0x10
    mov al, 0x69     ; 'i'
    int 0x10
    mov al, 0x6E     ; 'n'
    int 0x10
    mov al, 0x67     ; 'g'
    int 0x10
    mov al, 0x2E     ; '.'
    int 0x10
    int 0x10
    int 0x10
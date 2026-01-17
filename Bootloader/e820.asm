[bits 16]


E820:
    ;saving them both
	push ds
	push es
	
	xor ax, ax
	mov es, ax
	mov ds, ax
	
	mov di, 0x8004          ; destination
	xor ebx, ebx            ; continuation value starts at 0
	xor bp, bp              ; entry counter
	mov edx, 0x534D4150     ; 'SMAP' signature
	mov eax, 0xE820
	mov dword [es:di + 20], 1
	mov ecx, 24
	int 0x15
	jc .failed
	mov edx, 0x534D4150
	cmp eax, edx
	jne .failed
	test ebx, ebx
	je .failed
	jmp .jmpin
	
    ; a loop for e820 that pretty much counts usable memory regions
    .e820loop:
        mov eax, 0xE820
        mov dword [es:di + 20], 1
        mov ecx, 24
        int 0x15
        jc .e820f
        mov edx, 0x534D4150
        
    .jmpin:
        jcxz .skipent
        cmp cl, 20
        jbe .notext
        test byte [es:di + 20], 1
        je .skipent
        
    .notext:
        mov ecx, [es:di + 8]
        or ecx, [es:di + 12]
        jz .skipent
        inc bp
        add di, 24
        
    .skipent:
        test ebx, ebx
        jne .e820loop
        
    .e820f:
            mov word [0x8000], bp
            
            mov ax, bp
            
            pop es
            pop ds
            clc
            ret
        
    .failed:
        pop es
        pop ds
        stc
        ret
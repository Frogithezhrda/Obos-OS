
[BITS 32]

ataReadSectors:
    pusha
    ; ports 1F0h-1F7h
    call WaitBSY
    mov dx, 1F2h
    mov al, bl
    out dx, al
    
    mov al, cl
    mov dx, 1F3h
    out dx, al  
    mov al, ch
    mov dx, 1F4h
    out dx, al
    shr ecx, 16
    mov al, cl
    mov dx, 1F5h
    out dx, al
    mov dx, 1F6h
    mov al, 40h
    out dx, al
    mov dx, 1F7h
    mov al, 20h
    out dx, al 
    
    xor ecx, ecx
    mov cl, bl
.read:
    call WaitBSY
    call WaitDrq
    
    push ecx
    
    mov ecx, 256
    mov dx, 1F0h
    
.read_word:
    in ax, dx
    stosw               
    loop .read_word
    
    call Delay
    
    pop ecx
    loop .read
    
    popad
    ret

WaitBSY:
    mov dx, 1F7h
    in al, dx
    and al, 80h
    jnz WaitBSY
    ret

WaitDrq:
    mov dx, 1F7h
    in al, dx
    and al, 40h
    jz WaitDrq
    ret

Delay:
    mov dx, 3F6h
    in al, dx
    in al, dx 
    in al, dx 
    in al, dx 
    ret
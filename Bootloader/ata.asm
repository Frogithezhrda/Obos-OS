
[BITS 32]
;Reads one or more disk sectors using ATA
;ATA (Advanced Technology Attachment) is an old but simple 
; standard for talking to hard drives
; the disk has A small set of registers exposed to the CPU via I/O ports
; its like a tiny computer that the CPU can talk to about disk operations
ataReadSectors:
    ;preserving registers
    pusha
    ; ports 1F0h-1F7h
    call WaitBSY ; waiting for disk to be ready
    
    ;sector count setup
    mov dx, 1F2h ; sector count port
    mov al, bl ; bl ahs sector count
    out dx, al ; write sector count to port
    
    ;lba setup
    ;LBA — Logical Block Addressing.
    ;its a way to specify the location of blocks of data stored
    ; on computer storage devices
    ; we use 28-bit LBA addressing here
    ; like a 28 bit address for the disk
    ; first the low byte, then middle, then high byte
    ; and 24-27 bits go to drive/head register
    mov al, cl ; low byte of sector number
    mov dx, 1F3h ; sector number port
    out dx, al  ; write to port

    mov al, ch ; middle byte of sector number
    mov dx, 1F4h ; cylinder low port
    out dx, al ; write to port

    shr ecx, 16 ; shift right to get high byte of sector number
    mov al, cl ; high byte of sector number
    mov dx, 1F5h ; cylinder high port
    out dx, al ; write to port

    ;drive/head setup
    mov dx, 1F6h ;select drive/head register
    mov al, 40h ;select master drive
    out dx, al ; write to port

    ;send read command
    mov dx, 1F7h ;command port
    mov al, 20h ;read sectors command
    out dx, al  ; send command
    
    xor ecx, ecx ;cleaning
    mov cl, bl ;sector count in cl
.read:
    call WaitBSY ;waiting for disk to be ready
    call WaitDrq ;waiting for data request
    
    push ecx ;preserving sector count
    ;reading 256 words (512 bytes) per sector
    mov ecx, 256
    mov dx, 1F0h ; data port
    
.read_word:
    ; read word from data port
    in ax, dx ; read word
    ; store word to memory
    ; assuming destination address is in edi
    stosw               
    loop .read_word ; repeat for 256 words
    
    call Delay ; small delay after reading sector for stability
    
    pop ecx ;restoring sector count
    loop .read ; repeat for all sectors
    
    popad ;restoring registers
    ret

; to wait until the BSY (busy) bit is cleared
; because the disk sets this bit when it is busy processing a command
; and clears it when it is ready for the next command
WaitBSY:
    mov dx, 1F7h ; status port
    in al, dx ; read status
    and al, 80h ; check BSY bit
    jnz WaitBSY ; wait while BSY is set
    ret

; wait until the DRQ (data request) bit is set
; because the disk sets this bit when it is ready to transfer data
; and clears it when it is not ready
WaitDrq:
    mov dx, 1F7h ; status port
    in al, dx ; read status
    and al, 40h ;  check DRQ bit
    jz WaitDrq ; wait while DRQ is not set
    ret

;because some disks need a small delay after operations
;race conditions bullshit but it didnt work without it
Delay:
    mov dx, 3F6h ; alternate status port
    in al, dx ; read status
    in al, dx 
    in al, dx 
    in al, dx
    ; this causes a small delay
    ret
global ExceptionHandlers
extern exceptionHandler
extern timerISR
extern keyboardISR
extern rtcISR
extern pageFaultHandler

section .text
%macro ISR_NOERR 1
isr%1:
    push byte 0  
    push byte %1       
    jmp IsrCommon
%endmacro

%macro ISR_ERR 1
isr%1:
    push byte %1 
    jmp IsrCommon
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8    
ISR_NOERR 9
ISR_ERR   10   
ISR_ERR   11   
ISR_ERR   12   
ISR_ERR   13   
ISR_ERR   14   
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17   
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30   
ISR_NOERR 31
global isr32
global isr33
global isr40
global pageFaultISR

isr32:
    cli
    push byte 0
    push byte 32
    call timerISR
    add esp, 8
    sti
    iret

isr33:
    cli
    push byte 0
    push byte 33
    call keyboardISR
    add esp, 8
    sti
    iret

isr40:
    cli
    push byte 0
    push byte 40
    call rtcISR
    add esp, 8
    sti
    iret

IsrCommon:
    pusha
    mov eax, [esp + 32]
    push eax
    call exceptionHandler
    add esp, 4 
    popa
    add esp, 8
    iret

pageFaultISR:
    cli
    ; error code is 12 bytes above current ESP: EIP(4) + CS(4) + EFLAGS(4)
    mov eax, [esp]  ; read error code
    push eax
    call pageFaultHandler
    add esp, 4           ; clean up pushed error code
    sti
    iret

section .data
ExceptionHandlers:
    dd isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
    dd isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
    dd isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
    dd isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    dd isr32, isr33, 0, 0, 0, 0, 0, isr40
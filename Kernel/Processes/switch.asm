[BITS 32]

global switch_to
global start_first_process

section .text

; void switch_to(uint32_t* old_esp, uint32_t new_esp)
switch_to:
    ; Save state of current process
    push ebp
    push edi
    push esi
    push ebx

    ; Save current sp into the old PCB
    ; (Normally old_esp will be +4. and new_esp will be +8, but we pushed 4 registers so they increased by 16)
    mov eax, [esp + 20]
    mov [eax], esp      ; *old_esp = esp

    ; Switch to new stack
    mov esp, [esp + 24] ; esp = new_esp

    ; Restore state of new process
    pop ebx
    pop esi
    pop edi
    pop ebp

    ; Jump to the code (pops eip aka.entryPoint from the stack into eip reg)
    ret

; void start_first_process(uint32_t new_esp)
start_first_process:
    ; Switch to new stack
    mov esp, [esp + 4]

    ; Restore state of new process (here it will always be the fake ones.. )
    pop ebx
    pop esi 
    pop edi
    pop ebp
    
    ; Jump to the code
    ret

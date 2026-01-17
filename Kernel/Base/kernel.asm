[BITS 32]

EXTERN obos_main

section .text

global _start


_start:

    call obos_main

    Halt:
        hlt
        jmp Halt
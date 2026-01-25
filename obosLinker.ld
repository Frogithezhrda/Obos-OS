OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    
    .text : ALIGN(4096)
    {
        *(.multiboot)
        *(.text)
    }
    
    .rodata : ALIGN(4096)
    {
        *(.rodata.*)
    }
    
    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }
    . = ALIGN(4096);  /* Align to page boundary */
    _kernel_end = .;
}
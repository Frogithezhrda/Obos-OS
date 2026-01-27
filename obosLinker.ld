OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    
    _kernel_physical_start = .;
    
    .text : ALIGN(4096)
    {
        *(.multiboot)
        *(.text)
    }
    
    .rodata : ALIGN(4096)
    {
        *(.rodata*)
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
    
    . = ALIGN(4096);
    _kernel_physical_end = .;
}
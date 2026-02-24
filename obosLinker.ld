OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    
    _kernel_physical_start = .;
    
    .text : ALIGN(1024)
    {
        *(.multiboot)
        *(.text)
    }
    
    .rodata : ALIGN(1024)
    {
        *(.rodata*)
    }
    
    .data : ALIGN(1024)
    {
        *(.data)
    }
    
    .bss : ALIGN(1024)
    {
        *(COMMON)
        *(.bss)
    }
    
    . = ALIGN(1024);
    _kernel_physical_end = .;
}
OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    . = 0x10000;
    
    .text : ALIGN(512)
    {
        *(.text)
    }
    
    .rodata : ALIGN(512)
    {
        *(.rodata)
    }
    
    .data : ALIGN(512)
    {
        *(.data)
    }
    
    .bss : ALIGN(512)
    {
        *(COMMON)
        *(.bss)
    }
}
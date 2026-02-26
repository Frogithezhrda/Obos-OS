    #include "errors.h"

    const char* exceptionNames[EXCEPTION_COUNT] = {
        "Divide By Zero!!!",
        "Debug",
        "Mask Interrupt",
        "Breakpoint",
        "Overflow",
        "Bound Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault Exception",
        "Coprocessor Segment Overrun",
        "Unknown Exception",
        "Segment Not Avilable",
        "Stack Fault",  
        "General Protection Fault",
        "Page Fault",
        "Reserved",
        "FPU Floating Point Error",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating Point Exception",
        "Virtualization Exception",
        "Unknown Exception",
        "Unknown Exception",
        "Unknown Exception",
        "Unknown Exception",
        "Unknown Exception",
        "Unknown Exception",
        "Unknown Exception",
        "Hypervisor Injection Exception",
        "VMM Communication Exception",
        "Security Exception",
        "Unknown Exception"
    };

    void exceptionHandler(const unsigned char id)
    {
        kernelPanic(exceptionNames[id]);
    }


    void kernelPanic(const char* kernel)
    {
        // asm volatile ("cli");
        // // clearScreen();
        // print("KERNEL PANIC! ! !: ", RED);
        // printLine(kernel, LIGHT_RED);
        while(1) asm volatile ("hlt");
    }
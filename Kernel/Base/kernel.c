#include "kernel.h"

#define HIGHER_HALF_STACK_TOP 0xC0200000

void obosVirtualMain(void);

/*
Few Things to do that in general i didnt do yet that i want to do
Mark Warnings with
Warning: <message> - LIGHT_BROWN color
Error: <message> - red color
Info: <message> - light blue color
Those are for messages


*/

void obos_main()
{
    //basic initalization
    disableInterrupts();
    setupIDT();
    initalizeException();
    initializePIC();
    loadIDT();      
    initializeTimer();
    // maskAllInterrupts(); //no need to maksk interrupts here
    enableInterrupts();
    clearScreen();

        
    initializeMemoryManager();
    // printMemoryManagerInfo();
    initializePaging();
    enablePagingNow();
    printLine("Paging enabled successfully!", GREEN);

    /*
    we have virtual address space starting from 0xC0000000
    so we need to jump to that address space
    the kernel is loaded at 0x00100000 physical address
    the same with the stack virtual address is at 0xC0200000
    so we need to go that address space
    */
    unsigned int offset =(unsigned int)obosVirtualMain - 0x100000; 
    void (*entry)(void) =(void (*)(void))(KERNEL_START_ADDRESS + offset);
    asm volatile(
        "movl %[stack_top], %%esp\n\t"
        :
        : [stack_top] "r"(HIGHER_HALF_STACK_TOP)
    );

    entry();
    while (1);
}
void obosVirtualMain(void)
{
    clearScreen();
    printLine("Now running in higher-half kernel!", LIGHT_BLUE);

    unsigned int dummy;
    print("Address of dummy variable: ", LIGHT_BLUE);
    printNumber((unsigned int)&dummy, LIGHT_BLUE);
    print("\n", LIGHT_BLUE);
    while(1);
}
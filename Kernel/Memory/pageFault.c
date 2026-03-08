#include "pageFault.h"



void pageFaultHandler(unsigned int errorCode)
{
    unsigned int faultAddr;
    asm volatile("mov %%cr2, %0" : "=r"(faultAddr));
    register unsigned int nestedLevel asm("eax");
    nestedLevel++; //increments safely in a register, never touches memory

    // if(nestedLevel > 2)
    // {
    //     clearScreen();
    //     printLine("=== DOUBLE FAULT DETECTED ===", RED);
    //     while(1) asm volatile("hlt");
    // }
    printLine("\n=== PAGE FAULT ===", RED);
    
    print("Faulting address: 0x", WHITE);
    printHexNumber(faultAddr, WHITE);
    printLine("", WHITE);
    
    print("Error code: ", WHITE);
    printHexNumber(errorCode, WHITE);
    printLine("", WHITE);

    //basic demand paging handler
    if(!(errorCode & PF_PRESENT) && faultAddr >= USER_SPACE_START && faultAddr < USER_SPACE_END)
    {
        //this removes page offsets...
        unsigned int pageAddr = faultAddr & 0xFFFFF000;

        unsigned long long physFrame = allocateFreeFrame();
        if (physFrame == ERROR) kernelPanic("Out of memory!");
        
        if (physFrame < (MAPPED_MEMORY_MB_KERNEL * MB))
        {
            memset((void*)(unsigned int)physFrame, 0, PAGE_SIZE);
        }
        
        mapPage(pageAddr, (unsigned int)physFrame, 0);

        return;
    }

    if (errorCode & PF_PRESENT)
    {
        printLine("  [P] Page was present (protection violation)", RED);
    } 
    else 
    {
        printLine("  [P] Page not present", LIGHT_BLUE);
    }
    
    if (errorCode & PF_WRITE)
    {
        printLine("  [W/R] Access was a WRITE", RED);
    } 
    else 
    {
        printLine("  [W/R] Access was a READ", LIGHT_BLUE);
    }
    
    if (errorCode & PF_USER) 
    {
        printLine("  [U/S] User mode access", LIGHT_BLUE);
        //When we will have user mode processes we can add more info here
        //and kill the process that caused the fault
    } 
    else 
    {
        printLine("  [U/S] Supervisor mode access", LIGHT_BLUE);
        //Kernel Panic
        // kernelPanic("Page fault in supervisor mode!");
    }
    
    if (errorCode & PF_RESERVED) 
    {
        printLine("  [R] Reserved bit violation", RED);
    }
    
    if (errorCode & PF_INSTRUCTION) 
    {   
        printLine("  [I/D] Instruction fetch", LIGHT_BLUE);
    }
    
    printLine("==================", RED);
    
    asm volatile("cli");
    while (1);
}
#include "kernel.h"

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

    enterUserMode((void*)USER_SPACE_START);
    // // unsigned int* ptr = (unsigned int*)0xDEADBEEF;
    // unsigned int value = *ptr; //this will cause a page fault
    while (1);
}
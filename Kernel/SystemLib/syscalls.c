#include "syscalls.h"


void syscallHandler(const unsigned int syscallNumber, unsigned int arg1, unsigned int arg2)
{
    switch(syscallNumber)
    {
        case SYSCALL_PRINT:
        printW("SYSCALL_PRINT called with arg1=");
        printNumberW(arg1);
        printW("\n");
        
        // Try to read first byte
        char firstChar = *((char*)arg1);
        printW("First char: ");
        printCharW(firstChar);
        printW("\n");
        
        printW((const char*)arg1);
        break;
        case SYSCALL_GET_TICKS:
            if(arg1)
            {
                *(unsigned int*)arg1 = getTicks();
                printW("Current Ticks: ");
                printNumberW(arg1);
            }
            break;
        default:
            printLine("Warning: Unknown syscall", YELLOW);
            break;
    }
}
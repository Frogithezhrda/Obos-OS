#include "syscalls.h"


void syscallHandler(const unsigned int syscallNumber, unsigned int arg1, unsigned int arg2)
{
    switch(syscallNumber)
    {
        case SYSCALL_PRINT: 
            if(arg2 == 0)
                printLineW((const char*)arg1);
            else
                printLine((const char*)arg1, arg2);
        
        break;
        case SYSCALL_GET_TICKS:
            if(arg1)
            {
                unsigned int ticks = getTicks();
                *(unsigned int*)arg1 = ticks;
                printW("Current Ticks: ");
                printNumberW(ticks);
            }
            break;
        case SYSCALL_MALLOC:
            
            break;

        case SYSCALL_FREE:

            break;

        case SYSCALL_SLEEP:
            asm volatile ("sti"); //got to enable interrupts
            if(arg1) sleep(arg1);
            break;
        default:
            printLine("Warning: Unknown syscall", YELLOW);
            break;
    }
}
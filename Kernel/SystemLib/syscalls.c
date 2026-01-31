#include "syscalls.h"


void syscallHandler(const unsigned int syscallNumber, unsigned int arg1, unsigned int arg2)
{
    switch(syscallNumber)
    {
        case SYSCALL_PRINT:
            // arg1 = pointer to string in user space
            printW((const char*)arg1);
            break;

        case SYSCALL_GET_TICKS:
            if(arg1)
            {
                *(unsigned int*)arg1 = getTicks();
            }
            break;

        default:
            printLine("Warning: Unknown syscall", YELLOW);
            break;
    }
}
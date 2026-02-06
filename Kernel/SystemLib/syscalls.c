#include "syscalls.h"


unsigned int syscallHandler(const unsigned int syscallNumber, unsigned int arg1, unsigned int arg2)
{
    switch(syscallNumber)
    {
        case SYSCALL_PRINT: 
            if(arg2 == 0)
                printLineW((const char*)arg1);
            else
                printLine((const char*)arg1, arg2);
            return SUCCESS;
            break;
        case SYSCALL_GET_TICKS:
            return getTicks();
            break;
        case SYSCALL_MALLOC:
            return (unsigned int)umalloc(arg1);
            break;
        case SYSCALL_FREE:
            ufree((void*)arg1);
            break;
        case SYSCALL_HOUR:
            if(arg1)
            {
                *(Time*)arg1 = getRTCTime();
                return SUCCESS;
            }
            return FAILED;
            break;
        case SYSCALL_SLEEP:
            asm volatile ("sti"); //got to enable interrupts
            if(arg1) sleep(arg1);
            return SUCCESS;
            break;
        default:
            printLine("Warning: Unknown syscall", YELLOW);
            return FAILED;
    }
}
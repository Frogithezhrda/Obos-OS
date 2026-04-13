#include "syscalls.h"


unsigned int syscallHandler(const unsigned int syscallNumber, unsigned int arg1, unsigned int arg2)
{
    switch(syscallNumber)
    {
        case SYSCALL_PRINT: 
            if(arg2 == 0)
                printLineW((const char*)arg1);
            else
            {
                Color color = {255, 255, 255};
                print((const char*)arg1, color);
            }
            return SUCCESS;
            break;
        case SYSCALL_INPUT:
            if (arg1 && arg2)
            {
                char c = keybosChar();
                if (c == 0) return 0; //nothing yet

                ((char*)arg1)[0] = c;
                // ((char*)arg1)[1] = '\0';
                return SUCCESS;
            }
            return FAILED;
        case SYSCALL_GET_TICKS:
            return getTicks();
            break;
        case SYSCALL_MALLOC:
            return (unsigned int)umalloc(arg1);
            break;
        case SYSCALL_FREE:
            ufree((void*)arg1);
            break;
        case SYSCALL_EXIT:
            // restoreKernelFrame();
            
            break;
        case SYSCALL_HOUR:
            if(arg1)
            {
                *(Time*)arg1 = getRTCTime();
                return SUCCESS;
            }
            return FAILED;
            break;
        case SYSCALL_CREATE_FILE:
            if(arg1)
            {
                createFileF((const char*)arg1);
                return SUCCESS;
            }
            return FAILED;
            break;
        case SYSCALL_CREATE_DIRECTORY:
            if(arg1)
            {
                createDir((const char*)arg1);
                return SUCCESS;
            }
            return FAILED;
            break;
        case SYSCALL_DELETE_FILE:
            if(arg1)
            {
                deleteFile((const char*)arg1);
                return SUCCESS;
            }
            return FAILED;
            break;
        case SYSCALL_READ_FILE:
            //todo
            if(arg1 && arg2)
            {
                return readFile((const char*)arg1, (char*)arg2, 144);
            }
            return FAILED;
            break;
        case SYSCALL_WRITE_FILE:
            if(arg1 && arg2)            
            {
                writeFile((const char*)arg1, (const char*)arg2, strlen((const char*)arg2));
                return SUCCESS;
            }
            return FAILED;
            break;
        case SYSCALL_SLEEP:
            if(arg1) sleep(arg1);
            return SUCCESS;
            break;
        case SYSCALL_RAND:
            srand(getTicks());
            return rand();
            break;
        default:
            printLine("Warning: Unknown syscall", YELLOW);
            return FAILED;
    }
}
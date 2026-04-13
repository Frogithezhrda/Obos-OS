#include "programRegistry.h"

extern unsigned char _binary_hello_bin_start[];
extern unsigned char _binary_hello_bin_end[];

//the program registry
UserProgram programs[] =
{
    { "hello", _binary_hello_bin_start, _binary_hello_bin_end }
    // {
    //     "shell",
    //     _binary_shell_bin_start,
    //     _binary_shell_bin_end - _binary_shell_bin_start
    // }
};

int programCount = sizeof(programs) / sizeof(UserProgram);



UserProgram* findProgram(const char* name)
{
    for (int i = 0; i < programCount; i++)
    {
        if (strcmp(programs[i].name, name) == 0)
        {
            return &programs[i];
        }
    }
    return 0;
}

void run(const char* name)
{
    UserProgram* prog = findProgram(name);

    if (!prog)
    {
        printLine("Program not found", RED);
        return;
    }

    //copy program into user memory
    unsigned int size = prog->end - prog->start;
    memcpy((void*)USER_SPACE_START, prog->start, size);

    void* resume = &&resumePoint;
    //jump to user mode
    enterUserMode((void*)USER_SPACE_START, resume);


    resumePoint: asm volatile("nop");
}


#include "ObosSys.h"

// SYSCALL_PRINT (0)
void printo(const char* string, unsigned int color)
{
    asm volatile(
        "mov $0, %%eax\n"
        "mov %0, %%ebx\n"        // arg1 = string
        "mov %1, %%ecx\n"        // arg2 = color
        "int $0x80\n"
        :
        : "r"(string), "r"(color)
        : "eax", "ebx", "ecx"
    );
}

// SYSCALL_GET_TICKS (1)
unsigned int getObosTicks()
{
    unsigned int result;
    asm volatile(
        "mov $1, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"        // get return value
        : "=r"(result)
        :
        : "eax"
    );
    return result;
}

// SYSCALL_MALLOC (5)
void* malloc(unsigned int size)
{
    void* result;
    asm volatile(
        "mov $5, %%eax\n"
        "mov %1, %%ebx\n"        // arg1 = size
        "int $0x80\n"
        "mov %%eax, %0\n"        // get return value (pointer)
        : "=r"(result)
        : "r"(size)
        : "eax", "ebx"
    );
    return result;
}

// SYSCALL_FREE (6)
void free(void* ptr)
{
    asm volatile(
        "mov $6, %%eax\n"
        "mov %0, %%ebx\n"        // arg1 = pointer
        "int $0x80\n"
        :
        : "r"(ptr)
        : "eax", "ebx"
    );
}

// SYSCALL_HOUR (8)
unsigned int getObosTime(Time* timePtr)
{
    int result;
    asm volatile(
        "mov $8, %%eax\n"
        "mov %1, %%ebx\n"        // arg1 = pointer to Time struct
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(result)
        : "r"(timePtr)
        : "eax", "ebx"
    );
    return result;
}

// SYSCALL_SLEEP (7)
void sleepo(unsigned int ms)
{
    asm volatile(
        "mov $7, %%eax\n"
        "mov %0, %%ebx\n"
        "int $0x80\n"
        :
        : "r"(ms)
        : "eax", "ebx"
    );
}
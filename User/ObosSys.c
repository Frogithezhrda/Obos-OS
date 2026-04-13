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

// SYSCALL_INPUT
unsigned int inputo(char* buffer, unsigned int size)
{
    asm volatile(
        "mov $2, %%eax\n"
        "mov %0, %%ebx\n"
        "mov %1, %%ecx\n"
        "int $0x80\n"
        :
        : "r"(buffer), "r"(size)
        : "eax", "ebx", "ecx"
    );
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

void exit()
{
    asm volatile(
        "mov $3, %%eax\n"
        "int $0x80\n"
        :
        :
        : "eax"
    );
}

unsigned int createFileo(const char* filename)
{
    int result;
    asm volatile(
        "mov $9, %%eax\n"
        "mov %1, %%ebx\n"        // arg1 = filename
        "int $0x80\n"
        "mov %%eax, %0\n"        // get return value
        : "=r"(result)
        : "r"(filename)
        : "eax", "ebx"
    );
    return result;
}
unsigned int deleteFileo(const char* filename)
{
    int result;
    asm volatile(
        "mov $10, %%eax\n"
        "mov %1, %%ebx\n"        // arg1 = filename
        "int $0x80\n"
        "mov %%eax, %0\n"        // get return value
        : "=r"(result)
        : "r"(filename)
        : "eax", "ebx"
    );
    return result;
}
unsigned int createDiro(const char* dirname)
{
    int result;
    asm volatile(
        "mov $14, %%eax\n"
        "mov %1, %%ebx\n"        // arg1 = dirname
        "int $0x80\n"
        "mov %%eax, %0\n"        // get return value
        : "=r"(result)
        : "r"(dirname)
        : "eax", "ebx"
    );
    return result;
}
unsigned int readFileo(const char* filename, char* buffer, unsigned int size)
{
    int result;
    asm volatile(
        "mov $12, %%eax\n"
        "mov %1, %%ebx\n"        // arg1 = filename
        "mov %2, %%ecx\n"        // arg2 = buffer
        "mov %3, %%edx\n"        // arg3 = size
        "int $0x80\n"
        "mov %%eax, %0\n"        // get return value (bytes read)
        : "=a"(result)
        : "r"(filename), "r"(buffer), "r"(size)
        : "ebx", "ecx", "edx"
    );
    return result;
}
unsigned int writeFileo(const char* filename, const char* data, unsigned int size)
{
    int result;
    asm volatile(
        "mov $13, %%eax\n"
        "mov %1, %%ebx\n"        // arg1 = filename
        "mov %2, %%ecx\n"        // arg2 = data
        "mov %3, %%edx\n"        // arg3 = size
        "int $0x80\n"
        "mov %%eax, %0\n"        // get return value (bytes written)
        : "=a"(result)
        : "r"(filename), "r"(data), "r"(size)
        : "ebx", "ecx", "edx"
    );
    return result;
}

unsigned int rando()
{
    int result;
    asm volatile(
        "mov $15, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(result)
        :
        : "eax"
    );
    return result;
}

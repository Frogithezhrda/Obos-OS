#include "ObosSys.h"


void printo(const char* string, unsigned int color)
{
    asm volatile(
        "mov $0, %%eax\n"
        "mov %0, %%ebx\n"
        "mov %1, %%ecx\n"
        "int $0x80\n"
        :
        : "r"(string), "r"(color)
        : "eax", "ebx", "ecx"
    );
}
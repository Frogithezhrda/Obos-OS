#include "obosMemory.h"

void* memset(const void* dest, const int value, unsigned int size)
{
    unsigned char* pointer = dest;
    while (size--) 
    {
        *pointer++ = (unsigned char)value;
    }
    return dest;
}
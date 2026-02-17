#include "obosMemory.h"

void* memset(void* dest, int value, unsigned int size)
{
    unsigned char* pointer = dest;
    while (size--) 
    {
        *pointer++ = (unsigned char)value;
    }
    return dest;
}


int strcmp(const char* str1, const char* str2)
{
    int i = 0;
    
    while (str1[i] != '\0' && str2[i] != '\0')
    {
        if (str1[i] != str2[i])
        {
            return str1[i] - str2[i];
        }
        i++;
    }
    return str1[i] - str2[i];
}

void* memcpy(const void* dest, const void* src, unsigned int size)
{
    char *d = dest;
    const char *s = src;
    while (size--)
    {
        *d++ = *s++;
    }
    return dest;
}
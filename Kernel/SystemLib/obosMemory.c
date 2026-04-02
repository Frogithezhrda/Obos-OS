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

char* strtok(char *str, const char *delim)
{
    static char* saved = NULL;
    if (str != NULL) saved = str;
    if (saved == NULL) return NULL;

    while (*saved && strchr(delim, *saved)) saved++;
    if (*saved == '\0') return NULL;

    char* token = saved;

    while (*saved && !strchr(delim, *saved)) saved++;
    if (*saved) *saved++ = '\0';

    return token;
}

char* strchr(const char* str, int c)
{
    while (*str)
    {
        if (*str == (char)c) return (char*)str;
        str++;
    }
    return NULL;
}

unsigned int strlen(const char *str) {
    unsigned int length = 0;

    while (str[length] != '\0') {
        length++;
    }

    return length;
}
int atoi(char *p) {
    int k = 0;
    while (*p) {
        k = (k << 3) + (k << 1) + (*p) - '0';
        p++;
     }
     return k;
}
char* strcpy(char* destination, const char* source) {
    if(destination == NULL)
        return NULL;

    char *ptr = destination;  // pointer to destination str

    // copy char from source
    while(*source != '\0'){
        *destination = *source;
        destination++;
        source++;
    }

    *destination = '\0'; // add null terminator 

    return ptr;
}
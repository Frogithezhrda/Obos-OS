#ifndef OBOSMEMORY_H
#define OBOSMEMORY_H

#define NULL 0

void* memcpy(const void* dest, const void* src, unsigned int size);
void* memset(void* dest, int value, unsigned int size);
int strcmp(const char* str1, const char* str2);
char* strtok(char *str, const char *delim);
char* strchr(const char* str, int c);
unsigned int strlen(const char *str);
#endif
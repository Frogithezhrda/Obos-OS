#ifndef OBOSSYS_H
#define OBOSSYS_H


/*
a lib for syscalls a wrapper
*/
typedef struct Time
{
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;
    unsigned int ms;
} Time;

void printo(const char* string, unsigned int color);
unsigned int getObosTicks(void);
void* malloc(unsigned int size);
void free(void* ptr);
unsigned int getObosTime(Time* timePtr);
void sleepo(unsigned int ms);
unsigned int createFileo(const char* filename);
unsigned int deleteFileo(const char* filename);
unsigned int createDiro(const char* dirname);
unsigned int readFileo(const char* filename, char* buffer, unsigned int size);
unsigned int writeFileo(const char* filename, const char* data, unsigned int size);
unsigned int rando();
unsigned int inputo(char* buffer, unsigned int size);
void exit();

#define printoW(string) printo(string, 0)

#endif

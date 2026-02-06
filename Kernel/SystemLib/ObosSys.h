#ifndef OBOSSYS_H
#define OBOSSYS_H


/*
a lib for syscalls a wrapper
*/
#include "../Drivers/consoleDriver.h"
#include "../Drivers/timerDriver.h"

void printo(const char* string, unsigned int color);
unsigned int getObosTicks(void);
void* malloc(unsigned int size);
void free(void* ptr);
unsigned int getObosTime(Time* timePtr);
void sleepo(unsigned int ms);


#define printoW(string) printo(string, 0)

#endif
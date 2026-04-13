#ifndef PROGRAM_REGISTRY_H
#define PROGRAM_REGISTRY_H

#include "../Drivers/consoleDriver.h"

#define USER_SPACE_START 0x40000000


typedef struct UserProgram
{
    const char* name;
    unsigned char* start;
    unsigned char* end;
} UserProgram;


UserProgram* findProgram(const char* name);
void run(const char* name);


#endif
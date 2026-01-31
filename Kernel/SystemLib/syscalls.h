#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "../Drivers/consoleDriver.h"
#include "../Drivers/timerDriver.h"

//current syscall numbers can add more if needed
#define SYSCALL_PRINT      0
#define SYSCALL_GET_TICKS  1


void syscallHandler(const unsigned int syscallNumber, unsigned int arg1, unsigned int arg2);


#endif
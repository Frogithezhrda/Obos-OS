#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "../Drivers/consoleDriver.h"
#include "../Drivers/timerDriver.h"

//current syscall numbers more will be added
#define SYSCALL_PRINT      0
#define SYSCALL_GET_TICKS  1
#define SYSCALL_MALLOC     5
#define SYSCALL_FREE       6
#define SYSCALL_SLEEP      7

void syscallHandler(const unsigned int syscallNumber, unsigned int arg1, unsigned int arg2);


#endif
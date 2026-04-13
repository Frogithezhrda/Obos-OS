#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "../Drivers/consoleDriver.h"
#include "../Drivers/timerDriver.h"
#include "../Fs/superblock.h"
//current syscall numbers more will be added
#define SYSCALL_PRINT      0
#define SYSCALL_GET_TICKS  1
#define SYSCALL_INPUT      2
#define SYSCALL_EXIT 3
#define SYSCALL_MALLOC     5
#define SYSCALL_FREE       6
#define SYSCALL_SLEEP      7
#define SYSCALL_HOUR      8
#define SYSCALL_CREATE_FILE      9
#define SYSCALL_DELETE_FILE      10
#define SYSCALL_OPEN_FILE        11
#define SYSCALL_READ_FILE        12
#define SYSCALL_WRITE_FILE       13
#define SYSCALL_CREATE_DIRECTORY 14
#define SYSCALL_RAND    15

#define SUCCESS 1
#define FAILED  0

unsigned int syscallHandler(const unsigned int syscallNumber, unsigned int arg1, unsigned int arg2);


#endif
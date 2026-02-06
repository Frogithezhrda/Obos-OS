#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "../Memory/heap.h"

#define NULL 0
#define TIME_SLICE 5 //temp time slice

typedef enum States
{
    Ready = 0,
    Running = 1,
    Waiting = 2,
    Terminated = 3,
    Zombie = 4
} States;

//currently only if flag interrupts and first bit enabled(intel says it must be)
typedef enum Flags
{
    IF = 0x202
} Flags;
typedef struct CPUContext
{
    int eip;
    int esp;
    int ebp;
    int eax, ebx, ecx, edx, esi, edi;
    int eflags;
} CPUContext;



typedef struct PCB
{
    unsigned int processID;
    unsigned int parentPID;
    States state;
    //all registers
    CPUContext cpuContext;
    //page directory
    unsigned int pageDirectory;
    unsigned int kernelStack;
    unsigned int userStack;

    //for scheduling
    unsigned int timeSlice;

    //like a linked list has prev node and next one
    struct PCB* prev;
    struct PCB* next;
} PCB;

//defining a queue
typedef struct ProcessQueue
{
    PCB* head;
    PCB* tail;
    unsigned int count;
} ProcessQueue;

extern PCB* currentProcess;

extern ProcessQueue readyQueue;
extern ProcessQueue waitingQueue;
extern ProcessQueue terminatedQueue;

void initQueue(ProcessQueue* queue);
void push(ProcessQueue* queue, PCB* process);
PCB* pop(ProcessQueue* queue);
void remove(ProcessQueue* queue, PCB* process);

PCB* createProcess(void* entryPoint);
void exitProcess(const int exitCode);


#endif
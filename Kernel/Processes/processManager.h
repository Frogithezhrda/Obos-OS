#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "../Memory/heap.h"

#define NULL 0
#define TIME_SLICE 20

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


// CPU Context
// edi
// esi
// ebx
// ebp
// eflags;
    // This is pushed automatically by the C compiler's CALL instruction when switch_to() is called.
// eip

typedef struct PCB
{
    // Context Information
    // This points to the CURRENT top of the stack for this process.
    // When the process is running, this value is outdated.
    // When the process is paused, this points to a CPU Context on the stack.
    void* esp;

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
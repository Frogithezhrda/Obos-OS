#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "../Memory/heap.h"
#include "../SystemLib/obosint.h"

#define NULL 0
#define TIME_SLICE 20

#define EFLAGS_IF         0x00000200                    // Interrupt Enable
#define EFLAGS_RESERVED   0x00000002                    // Mandatory bit (historically, bit 1 is always 1. If it's 0, some old processors get grumpy)
#define DEFAULT_EFLAGS    (EFLAGS_IF | EFLAGS_RESERVED) // Default flags for a new process

typedef enum States
{
    Ready = 0,      // Sitting in the queue waiting for a turn.
    Running = 1,    // Currently owning the CPU.
    Waiting = 2     // Waiting for an input.
} States;

// Currently only if flag interrupts and first bit enabled(intel says it must be)
typedef enum Flags
{
    IF = 0x202
} Flags;


// Process Control Block
// This structure holds all the information about a process.
typedef struct PCB
{
    uint32_t esp;            // The current stack pointer (Saved during switch)
    uint32_t pid;            // Process ID
    States state;            // Process State
    
    void* pageTableIndex;    // CR3 register value for paging
    
    // Safety: Separate stacks
    uint32_t kernelStack;    // Base of the kernel stack
    uint32_t userStack;      // Base of the user stack (if you have one)
    
    uint32_t timeSlice;      // Ticks remaining for this process
    
    struct PCB* prev;
    struct PCB* next;
} PCB;

// Queue structure for managing processes
typedef struct ProcessQueue
{
    PCB* head;
    PCB* tail;
    uint32_t count;
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
void runToWaiting();
void exitProcess(const int exitCode);
void terminateProcess();


#endif
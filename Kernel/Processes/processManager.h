#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

// Lifecycle and Resource Management of Processes

#include "../Memory/heap.h"
#include "../SystemLib/obosint.h"
#include "processQueue.h"


// Process Control Block
// This structure holds all the information about a process.
typedef enum States
{
    Ready = 0,      // Sitting in the queue waiting for a turn.
    Running = 1,    // Currently owning the CPU.
    Waiting = 2,    // Waiting for an input.
    Terminated = 3  // Process wouldn't be scheduled again.
} States;

typedef struct PCB
{
    uint32_t esp;            // The current stack pointer (Saved during switch)
   
    pid_t pid;               // Process ID
    States state;            // Process State

    void* pageTableIndex;    // CR3 register value for paging
    uint32_t timeSlice;      // Ticks remaining for this process
    
    uint32_t stackBase;      // Base of the allocated stack (for cleanup)

    struct PCB* next;
    struct PCB* prev;
} PCB;

extern PCB* currentProcess;

extern ProcessQueue readyQueue;
extern ProcessQueue waitingQueue;
extern ProcessQueue terminatedQueue;

void initQueues();
PCB* createProcess(uint32_t entryPoint);
void exitProcess(const int exitCode);

#endif
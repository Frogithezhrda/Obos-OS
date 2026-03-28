#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

// Lifecycle and Resource Management of Processes

#include "../Memory/heap.h"
#include "../SystemLib/obosint.h"
#include "processQueue.h"

typedef enum States
{
    Ready = 0,      // Sitting in the queue waiting for a turn.
    Running = 1,    // Currently owning the CPU.
    Waiting = 2,    // Waiting for an input.
    Terminated = 3  // Process wouldn't be scheduled again.
} States;

extern PCB* currentProcess;

extern ProcessQueue readyQueue;
extern ProcessQueue waitingQueue;
extern ProcessQueue terminatedQueue;

void initQueues();
PCB* createProcess(uint32_t entryPoint);
void exitProcess(const int exitCode);

#endif
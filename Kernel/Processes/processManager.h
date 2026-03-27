#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

// Lifecycle and Resource Management of Processes

#include "../Memory/heap.h"
#include "../SystemLib/obosint.h"
#include "processQueue.h"


// Process Control Block
// This structure holds all the information about a process.

extern PCB* currentProcess;

extern ProcessQueue readyQueue;
extern ProcessQueue waitingQueue;
extern ProcessQueue terminatedQueue;

void initQueues();
PCB* createProcess(uint32_t entryPoint);
void exitProcess(const int exitCode);

#endif
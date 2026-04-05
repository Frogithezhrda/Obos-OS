#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

// Lifecycle and Resource Management of Processes

#include "../Memory/heap.h"
#include "processQueue.h"

extern PCB* currentProcess;  

extern ProcessQueue readyQueue;
extern ProcessQueue waitingQueue;
extern ProcessQueue terminatedQueue;

void initQueues();
PCB* createProcess(uint32_t entryPoint);
void exitProcess(const int exitCode);
void wakeupProcess(PCB* process);

#endif
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "processManager.h"

// The assembly function that performs the actual stack swap
extern void switch_to(uint32_t* old_esp, uint32_t new_esp);
extern void start_first_process(uint32_t new_esp);

void scheduler();
void loadFirstProcess();

void tick(void);
void yield(void);

void wakeupProcess(PCB* process);
void toWaiting();

#endif
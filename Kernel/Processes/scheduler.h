#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "processManager.h"

void initScheduler(void);

void contextSwitch(PCB* old_pcb, PCB* next_pcb);
void loadFirstProcess(PCB* pcb);

void nextProcess(void);
void tick(void);
void yield(void);

#endif
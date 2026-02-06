#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "processManager.h"

void initScheduler(void);

void nextProcess(void);
void tick(void);
void yield(void);

#endif
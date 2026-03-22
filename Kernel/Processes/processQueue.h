#ifndef PROCESS_QUEUE_H
#define PROCESS_QUEUE_H

#include "../SystemLib/obosint.h"

typedef struct PCB PCB;

typedef struct ProcessQueue
{
    PCB* head;
    PCB* tail;
    uint32_t size;
} ProcessQueue;

void initQueue(ProcessQueue* q);
void push(ProcessQueue* q, PCB* process);
PCB* pop(ProcessQueue* q);
int isQueueEmpty(ProcessQueue* q);
void removeProcess(ProcessQueue* q, PCB* process);

#endif
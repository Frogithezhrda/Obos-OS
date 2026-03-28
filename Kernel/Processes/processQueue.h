#ifndef PROCESS_QUEUE_H
#define PROCESS_QUEUE_H

#include "../SystemLib/obosint.h"
#define NULL 0
#define TIME_SLICE 20

#define EFLAGS_IF         0x00000200                    // Interrupt Enable
#define EFLAGS_RESERVED   0x00000002                    // Mandatory bit (historically, bit 1 is always 1. If it's 0, some old processors get grumpy)
#define DEFAULT_EFLAGS    (EFLAGS_IF | EFLAGS_RESERVED) // Default flags for a new process

typedef uint8_t pid_t; 

struct PCB;

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
#ifndef PROCESS_QUEUE_H
#define PROCESS_QUEUE_H

#include "../SystemLib/obosint.h"
#define NULL 0
#define TIME_SLICE 20

#define EFLAGS_IF         0x00000200                    // Interrupt Enable
#define EFLAGS_RESERVED   0x00000002                    // Mandatory bit (historically, bit 1 is always 1. If it's 0, some old processors get grumpy)
#define DEFAULT_EFLAGS    (EFLAGS_IF | EFLAGS_RESERVED) // Default flags for a new process

typedef uint8_t pid_t; 

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
#include "processManager.h"

PCB* currentProcess = 0;

ProcessQueue readyQueue;
ProcessQueue waitingQueue;
ProcessQueue terminatedQueue;

static unsigned int nextPID = 1;

void initQueue(ProcessQueue* queue)
{
    queue->count = NULL;
    queue->head = NULL;
    queue->tail = NULL;
}

void push(ProcessQueue* queue, PCB* process)
{
    process->next = NULL;
    //the previous process is the one that was last in the queue
    //and the next will be the one after this one
    process->prev = queue->tail; // queue has head tail and count
    if (queue->tail)
    {
        queue->tail->next = process;
    }
    else
    {
        queue->head = process;
    }

    queue->tail = process;
    queue->count++;
}

PCB* pop(ProcessQueue* queue)
{
    PCB* process = queue->head;
    if(!process) return NULL;

    queue->head = process->next;
    if (queue->head)
    {
        //the one that was before this one is set to null
        queue->head->prev = NULL;
    }
    else
    {
        //if there is no head then there wont be any tail either
        queue->tail = NULL;
    }
    //clearing pointers of the popped process

    process->next = NULL;
    process->prev = NULL;
    queue->count--;
    return process;
}

void remove(ProcessQueue* queue, PCB* process)
{
    if (process->prev)
    {
        process->prev->next = process->next;
    }
    else
    {
        queue->head = process->next;
    }

    if (process->next)
    {
        process->next->prev = process->prev;
    }
    else
    {
        queue->tail = process->prev;
    }
    //clearing pointers of the popped process
    process->next = NULL;
    process->prev = NULL;
    queue->count--;
}


PCB* createProcess(void* entryPoint)
{
    PCB* process = (PCB*)kmalloc(sizeof(PCB));
    if(!process) return NULL;

    process->processID = nextPID++;
    process->parentPID = currentProcess ? currentProcess->processID : 0;
    process->state = Ready;

    process->cpuContext.eip = (unsigned int)entryPoint;
    process->cpuContext.eflags = IF; 
    process->timeSlice = TIME_SLICE;

    /*
    need to add the stack and page directory
    */

    asm volatile (
        "mov %[new_esp], %%esp \n\t"  // Jump to the new stack
        "pushfl \n\t"
        "push %%ebp \n\t"
        "push %%ebx \n\t"
        "push %%esi \n\t"
        "push %%edi \n\t"
        : 
        : [new_esp] "m" (process->esp)
        : "memory"
    );

    push(&readyQueue, process);
}

void runToWaiting()
{
    if(!currentProcess) return;

    currentProcess->state = Waiting;
    push(&waitingQueue, currentProcess);
    currentProcess = NULL;
}

void exitProcess(const int exitCode)
{
    if(!currentProcess) return;

    // parents should be aware and kids should be down too?
    kfree(currentProcess);
    currentProcess = NULL;
}

void terminateProcess()
{
    if(!currentProcess) return;

    currentProcess->state = Terminated;
    push(&terminatedQueue, currentProcess);
    currentProcess = NULL;
}
#include "processManager.h"

PCB* currentProcess = 0;

ProcessQueue readyQueue;
ProcessQueue waitingQueue;
ProcessQueue terminatedQueue;

static unsigned int nextPID = 1;

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

// void terminateProcess()
// {
//     if(!currentProcess) return;

//     currentProcess->state = Terminated;
//     push(&terminatedQueue, currentProcess);
//     currentProcess = NULL;
// }
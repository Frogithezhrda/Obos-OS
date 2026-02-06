#include "scheduler.h"


void initScheduler()
{
    initQueue(&readyQueue);
    initQueue(&waitingQueue);
    initQueue(&terminatedQueue);
    currentProcess = NULL;
}

void nextProcess()
{
    PCB* previousProcess = currentProcess;
    currentProcess = pop(&readyQueue);

    if(!currentProcess) return;

    currentProcess->state = Running;

    //if the preivous process still alive need to save it in the context switch
    if(previousProcess)
    {
        //TODO Context switc
    }
    else //still need to switch probably to a process so
    {
        //TODO Context switc
    }
}

void tick()
{
    if(!currentProcess) return;

    currentProcess->timeSlice--;
    if(currentProcess->timeSlice <= 0)
    {
        yield(); 
    }
}

void yield()
{
    if(currentProcess)
    {
        currentProcess->state = Ready;
        currentProcess->timeSlice = TIME_SLICE;
        push(&readyQueue, currentProcess);
    }

    nextProcess();
}
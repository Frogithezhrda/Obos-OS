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

// CPU Context
// edi
// esi
// ebx
// ebp
// eflags;
    // This is pushed automatically by the C compiler's CALL instruction when switch_to() is called.
// eip
void switch_to(PCB* old_pcb, PCB* next_pcb) {
    asm volatile (
        // Save current context onto the current stack
        "pushfl \n\t"              // Push EFLAGS
        "push %%ebp \n\t"
        "push %%ebx \n\t"
        "push %%esi \n\t"
        "push %%edi \n\t"

        // Save current ESP into old_pcb->esp
        // We use %[old_esp] to let C tell us where that variable is
        "mov %%esp, %[old_esp] \n\t"

        // THE SWITCH: Load the new ESP from next_pcb->esp
        "mov %[next_esp], %%esp \n\t"

        // Restore new context from the new stack
        "pop %%edi \n\t"
        "pop %%esi \n\t"
        "pop %%ebx \n\t"
        "pop %%ebp \n\t"
        "popfl \n\t"               // Pop EFLAGS

        : [old_esp] "=m" (old_pcb->esp)  // Output: write hardware ESP here
        : [next_esp] "m" (next_pcb->esp) // Input: read new ESP from here
        : "memory"                       // Tell compiler memory has changed
    );
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
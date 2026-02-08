#include "scheduler.h"

enum Mode {SAVE_AND_SWITCH = 0, SWITCH_ONLY};

void initScheduler()
{
    initQueue(&readyQueue);
    initQueue(&waitingQueue);
    initQueue(&terminatedQueue);
    currentProcess = NULL;
}

// CPU Context
// edi
// esi
// ebx
// ebp
// eflags;
    // This is pushed automatically by the C compiler's CALL instruction when switch_to() is called.
// eip
void contextSwitch(PCB* old_pcb, PCB* next_pcb, Mode mode) {

    if (mode == SAVE_AND_SWITCH){
       saveContext(old_pcb);
       loadContext(next_pcb);
    }
    else (mode == SWITCH_ONLY){
       loadContext(next_pcb);
    }
    
}

void saveContext(PCB* pcb){
    asm volatile (
        // Save current context onto the current stack
        "pushfl \n\t"              // Push EFLAGS
        "push %%ebp \n\t"
        "push %%ebx \n\t"
        "push %%esi \n\t"
        "push %%edi \n\t"

        // Save current ESP into old_pcb->esp
        // We use %[old_esp] to let C tell us where that variable is
        "mov %%esp, %[pcb] \n\t"

        : [pcb] "=m" (pcb->esp)  // Output: write hardware ESP here
        : "memory"                       // Tell compiler memory has changed
    )
}

void loadContext(PCB* pcb){
    asm volatile (
        // THE SWITCH: Load the new ESP from next_pcb->esp
        "mov %[pcb], %%esp \n\t"

        // Restore new context from the new stack
        "pop %%edi \n\t"
        "pop %%esi \n\t"
        "pop %%ebx \n\t"
        "pop %%ebp \n\t"
        "popfl \n\t"               // Pop EFLAGS

        : [pcb] "m" (pcb->esp) // Input: read new ESP from here
        : "memory"                       // Tell compiler memory has changed
    )
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
        contextSwitch(previousProcess, currentProcess, SAVE_AND_SWITCH);
    }
    else //still need to switch probably to a process so
    {
        contextSwitch(previousProcess, currentProcess, );
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
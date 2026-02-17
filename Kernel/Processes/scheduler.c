#include "scheduler.h"

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
void contextSwitch(PCB* old_pcb, PCB* next_pcb) {
    asm volatile (
        // 1. SAVE: Only if we have an old process
        "pushfl \n\t"
        "push %%ebp \n\t"
        "push %%ebx \n\t"
        "push %%esi \n\t"
        "push %%edi \n\t"

        // Store the stack pointer into the OLD pcb
        "mov %%esp, %[old_esp] \n\t"

        // 2. SWITCH: Change the ground under the CPU's feet
        "mov %[next_esp], %%esp \n\t"

        // 3. LOAD: Restore registers from the NEW stack
        "pop %%edi \n\t"
        "pop %%esi \n\t"
        "pop %%ebx \n\t"
        "pop %%ebp \n\t"
        "popfl \n\t"

        : [old_esp] "=m" (old_pcb->esp) 
        : [next_esp] "m" (next_pcb->esp)
        : "memory"
    );
}

void loadFirstProcess(PCB* pcb)
{
    // Load the first process's stack pointer into ESP and jump to its entry point
    asm volatile (
        "mov %[next_esp], %%esp \n\t"
        "pop %%edi \n\t"
        "pop %%esi \n\t"
        "pop %%ebx \n\t"
        "pop %%ebp \n\t"
        "popfl \n\t"
        :
        : [next_esp] "m" (pcb->esp)
        : "memory"
    );
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
        contextSwitch(previousProcess, currentProcess);
    }
    else //still need to switch probably to a process so
    {
        loadFirstProcess(currentProcess);
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
    if(!currentProcess) return;
    
    currentProcess->state = Ready;
    currentProcess->timeSlice = TIME_SLICE;
    push(&readyQueue, currentProcess);

    nextProcess();
}
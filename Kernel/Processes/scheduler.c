#include "scheduler.h"

void scheduler(){
    PCB* old = currentProcess;

    // Get next process from ready queue
    PCB* next = pop(&readyQueue);

    if (next == NULL){
        return;
    }

    // Put old process back to ready queue
    if (old->state == Running){
        old->state = Ready;
        push(&readyQueue, old);
    }

    // Switch to the next process
    currentProcess = next;
    next->state = Running;

    switch_to(&(old->esp), next->esp);
}

void loadFirstProcess(PCB* pcb){
    // Pop the first process from the ready queue
    PCB* first = pop(&readyQueue);

    if (first == NULL) return;

    currentProcess = first;
    first->state = Running;

    start_first_process(first->esp);
}

// void initScheduler()
// {
//     initQueue(&readyQueue);
//     initQueue(&waitingQueue);
//     initQueue(&terminatedQueue);
//     currentProcess = NULL;
// }

// // CPU Context
// // edi
// // esi
// // ebx
// // ebp
// // eflags;
//     // This is pushed automatically by the C compiler's CALL instruction when switch_to() is called.
// // eip
// void contextSwitch(PCB* old_pcb, PCB* next_pcb) {
//     asm volatile (
//         // 1. SAVE: Only if we have an old process
//         "pushfl \n\t"
//         "push %%ebp \n\t"
//         "push %%ebx \n\t"
//         "push %%esi \n\t"
//         "push %%edi \n\t"

//         // Store the stack pointer into the OLD pcb
//         "mov %%esp, %old_esp \n\t"

//         mov %next_ptbr, %%eax \n\t"
//         "mov %%eax, %%cr3 \n\t" // Switch to the old process
        
//         // 2. SWITCH: Change the ground under the CPU's feet
//         "mov %next_esp, %%esp \n\t"

//         // 3. LOAD: Restore registers from the NEW stack
//         "pop %%edi \n\t"
//         "pop %%esi \n\t"
//         "pop %%ebx \n\t"
//         "pop %%ebp \n\t"
//         "popfl \n\t"

//         :
//         : [old_esp] "=m" (old_pcb->esp) 
//         : [next_esp] "m" (next_pcb->esp)
//         : [next_ptbr] "m" (next_pcb->pageTableIndex), "memory"
//         : "eax","memory"
//     );
// }

// void loadFirstProcess(PCB* pcb)
// {
//     // Load the first process's stack pointer into ESP and jump to its entry point
//     asm volatile (
//         "mov %[next_esp], %%esp \n\t"
//         "pop %%edi \n\t"
//         "pop %%esi \n\t"
//         "pop %%ebx \n\t"
//         "pop %%ebp \n\t"
//         "popfl \n\t"

//         :
//         : [next_esp] "m" (pcb->esp)
//         : "memory"
//     );
// }

// void nextProcess()
// {
//     PCB* previousProcess = currentProcess;
//     currentProcess = pop(&readyQueue);

//     if(!currentProcess) return;

//     currentProcess->state = Running;

//     //if the preivous process still alive need to save it in the context switch
//     if(previousProcess)
//     {
//         contextSwitch(previousProcess, currentProcess);
//     }
//     else //still need to switch probably to a process so
//     {
//         loadFirstProcess(currentProcess);
//     }
// }

// void tick()
// {
//     if(!currentProcess) return;

//     wakeupReadyWaitingProcesses();
//     currentProcess->timeSlice--;
//     if(currentProcess->timeSlice <= 0)
//     {
//         yield(); 
//     }
// }

// void yield()
// {
//     if(!currentProcess) return;

//     currentProcess->state = Ready;
//     currentProcess->timeSlice = TIME_SLICE;
//     push(&readyQueue, currentProcess);

//     nextProcess();
// }

// void wakeupReadyProcesses()
// {
//     PCB* process = waitingQueue.head;
//     PCB* nextProcess = process->next;

//     while (process != NULL){
//         // if (process->state != Ready) {
//         //     remove(&waitingQueue, process);
//         //     //delete process? if terminated?
//         //     process = nextProcess;
//         //     continue;
//         // }

//         remove(&waitingQueue, process);
//         process->state = Ready;

//         push(&readyQueue, process);
//         process = nextProcess;  
//     }
// }

// void wakeupWaitingToReadyProcesses(PCB* process)
// {
//     process->state = Ready;
//     remove(&waitingQueue, process);
//     push(&readyQueue, process);
// }
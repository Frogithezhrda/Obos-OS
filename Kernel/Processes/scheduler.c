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

void loadFirstProcess(){
    // Pop the first process from the ready queue
    PCB* first = pop(&readyQueue);

    if (first == NULL) return;

    currentProcess = first;
    first->state = Running;

    start_first_process(first->esp);
}

void tick(void){
    if (currentProcess == NULL) return;

    // Decrement the time slice of the current process
    if (currentProcess->timeSlice > 0){
        currentProcess->timeSlice--;
    }

    // If time slice is exhausted, schedule the next process
    if (currentProcess->timeSlice <= 0){
        currentProcess->timeSlice = TIME_SLICE; // Reset time slice
        scheduler(); // Switch to the next process
    }
}

void yield(void){
    if (currentProcess == NULL) return;

    currentProcess->timeSlice = TIME_SLICE; // Reset time slice
    scheduler(); // Switch to the next process
}

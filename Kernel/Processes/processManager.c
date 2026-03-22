#include "processManager.h"

PCB* currentProcess = NULL;

ProcessQueue readyQueue;
ProcessQueue waitingQueue;
ProcessQueue terminatedQueue;

static pid_t nextPID = 1;

#define STACK_SIZE 4096
#define REGISTERS_COUNT 4
#define FAKE_REGISTER_VALUE 0

void initQueues(){
    initQueue(&readyQueue);
    initQueue(&waitingQueue);
    initQueue(&terminatedQueue);
}

PCB* createProcess(uint32_t entryPoint)
{
    PCB* process = (PCB*)kmalloc(sizeof(PCB));
    if(!process) return NULL;

    // Allocate 4KB for the stack 
    uint32_t stackBase = (uint32_t)kmalloc(STACK_SIZE); 

    if(!stackBase) {
        kfree(process);
        return NULL;
    }
    
    // Start at the top
    uint32_t* sp = (uint32_t*)(stackBase + STACK_SIZE); 
    
    // When the assembly 'ret' runs, it pops this address into EIP.
    *(--sp) = (uint32_t)entryPoint;

    // Push the Fake Registers (Matching the assembly 'pop' order)
    for(int i = 0; i < REGISTERS_COUNT; i++) {
        *(--sp) = FAKE_REGISTER_VALUE;  // Fake values for the registers
    }

    // Set stack pointer to the top of the stack
    process->esp = (uint32_t)sp;  

    // Set the rest of the PCB fields
    process->pid = nextPID++;
    process->state = Ready;
    process->timeSlice = TIME_SLICE;
    process->stackBase = stackBase;

    // Safety initialization
    process->next = NULL;
    process->prev = NULL;

    push(&readyQueue, process);

    return process;
}

void exitProcess(const int exitCode) {
    // Mark process as terminated
    currentProcess->state = Terminated;

    // Move the process to the terminated queue
    push(&terminatedQueue, currentProcess);

    // Call the scheduler to pick the next process.
    schedule();

    // Halt if the scheduler fails to find a new process
    for(;;); 
}

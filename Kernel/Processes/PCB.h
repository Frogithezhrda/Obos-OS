#ifndef PCB_H
#define PCB_H

// Process Control Block
// This structure holds all the information about a process.
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

#endif
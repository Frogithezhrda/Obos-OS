#include "kernel.h"

/*
Few Things to do that in general i didnt do yet that i want to do
Mark Warnings with
Warning: <message> - yellow color
Error: <message> - red color
Info: <message> - light blue color
Those are for messages

BARAK ITS FOR YOU NOT FOR ME READ IT I USED CHAT FOR THIS ONE
IF YOU THINK SOMETHING IS A BUG ASK ME FIRST CONFIRM IT CAUSE IT MIGHT NOT BE!
AND IF YOU FIND IT AND CONFIRM PLEASE JUST FIX

A Small guide to using the OS parts:

------------------------------------------------------
1. Initialization Order

- disableInterrupts(): Temporarily stops CPU interrupts.
- setupIDT(): Prepare Interrupt Descriptor Table (IDT) entries.
- initalizeException(): Register handlers for CPU exceptions (divide by zero, page fault, GPF, etc.).
- initializePIC(): Setup the Programmable Interrupt Controller for hardware interrupts.
- loadIDT(): Load the IDT into the CPU.
- initializeTimer(): Setup system timer (optional for scheduling).
- enableInterrupts(): Start handling interrupts.
- clearScreen(): Clear VGA display for neat output.

------------------------------------------------------
2. Memory & Paging

- initializeMemoryManager(): Sets up free frame list and bookkeeping structures.
- initializePaging() + enablePagingNow():
    - Activates virtual memory.
    - Kernel memory is mapped (identity-mapped initially).
    - User memory is unmapped initially (demand paging applies).

- Demand Paging:
    - Only allocated when a user page is accessed and triggers a page fault.
    - Page fault handler checks:
        - If faulting address is in user space:
            1. Allocate a new physical frame.
            2. Zero it.
            3. Map it as user-accessible.
        - If fault is illegal (kernel access to user page, reserved bits, etc.), a protection fault occurs.

- Use faultAddr inside page fault handler to see which virtual address triggered the fault.
- Accessing an unmapped address in user space automatically allocates a page (if handled correctly).

------------------------------------------------------
3. Kernel Heap (kmalloc / kfree)

- Preallocated in kernel virtual memory (mapped during initKernelHeap).
- kmalloc(size):
    - Find a free block large enough.
    - Split block if it’s bigger than requested.
    - Return pointer **after the HeapBlock header**.
- kfree(ptr):
    - Marks block free.
    - Merge adjacent free blocks to reduce fragmentation.

- Example:
block layout: [HEADER][DATA]
Returned pointer = HEADER + HEAP_BLOCK_HEADER_SIZE
To get header later: (HeapBlock*)((char*)ptr - HEAP_BLOCK_HEADER_SIZE)
------------------------------------------------------
4. User Heap (umalloc / ufree)

- First page allocated on demand.
- umalloc(size) behaves like kmalloc, but pages are user-accessible.
- Must run in user mode to test real demand paging.
- Kernel can still write mapped pages, but accessing unmapped addresses in kernel triggers GPF if permissions wrong.
- Do not start executing directly in USER_HEAP_START — first page may not exist yet.

------------------------------------------------------
5. User Mode Execution

- enterUserMode(startAddr):
    - Switch CPU to ring 3 (user mode) and jump to startAddr.
    - Only code in USER_SPACE_START should run initially.
- Example of a minimal user program triggering demand paging:

code[0] = 0xA1; // MOV EAX, moffs32
(unsigned int)&code[1] = USER_SPACE_START + 0x5000; // triggers PF
code[4] = 0xEB; // JMP short
code[5] = 0xFE; // infinite loop

- Accessing unmapped addresses automatically triggers page faults handled by your OS.

------------------------------------------------------
6. Exceptions

- General Protection Fault (GPF): triggered when CPU detects illegal access (like kernel writing user memory with wrong permissions).
- Page Fault (PF):
    - Use errorCode to determine cause:
        - PF_PRESENT: page already present but protection violation.
        - PF_WRITE: write access.
        - PF_USER: user mode access.
        - PF_RESERVED: reserved bits in PDE/PTE violated.
        - PF_INSTRUCTION: instruction fetch.
    - Use faultAddr to map new pages in demand paging.

------------------------------------------------------
7. Notes / Best Practices

- Always test user heap in user mode.
- Print debug info when allocating pages for demand paging.
- Heap alignment ensures proper memory access.
- Don’t enter user mode pointing to uninitialized user heap.
- Small allocations will split blocks to avoid wasting memory.
AND DONT FORGET TO PUT THE COLOR WHEN CALLING THE PRINT
*/


void obos_main()
{
    //basic initalization
    disableInterrupts();
    setupIDT();
    initalizeException();
    initializePIC();
    loadIDT();      
    initializeTimer();
    // maskAllInterrupts(); //no need to maksk interrupts here
    enableInterrupts();
    clearScreen();

        
    initializeMemoryManager();
    // printMemoryManagerInfo();
    initializePaging();
    enablePagingNow();
    initKernelHeap();
    initUserHeap();

    // enterUserMode((void*)USER_SPACE_START);
    // // unsigned int* ptr = (unsigned int*)0xDEADBEEF;
    // unsigned int value = *ptr; //this will cause a page fault
    while (1);
}
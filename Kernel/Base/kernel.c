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

void shell()
{
    char* string = kmalloc(100);
    while(1)
    {
        print("\n>>", LIGHT_GREEN);
        keybos(string, 100);
        char* cmd = strtok(string, " ");

        if(cmd == NULL)
        {
            continue;
        }
        else if(!strcmp(cmd, "exit"))
        {
            printLineW("Exiting...");
            break;
        }
        else if(!strcmp(cmd, "ls"))
        {
            ls();
        }
        else if(!strcmp(cmd, "repeat"))
        {
            char* param = strtok(NULL, "\0");
            if(param != NULL) printW(param);
        }
        else if(!strcmp(cmd, "read"))
        {
            char* param = strtok(NULL, " ");
            if(param == NULL)
            {
                printLineW("Usage: read <filename>");
                continue;
            }
            char* buffer = (char*)kmalloc(144);
            readFile(param, buffer, 144);
            printW("File contents:\n");
            printW(buffer);
            kfree(buffer);
        }
        else if(!strcmp(cmd, "write"))
        {
            char* param = strtok(NULL, " ");
            if(param == NULL)
            {
                printLineW("Usage: write <filename>");
                continue;
            }
            char* data = strtok(NULL, "\0");
            if(data == NULL)
            {
                printLineW("Usage: write <filename> <data>");
                continue;
            }
            writeFile(param, data, strlen(data));
            printLineW("File written successfully!");
        }
        else if(!strcmp(cmd, "create"))
        {
            char* param = strtok(NULL, " ");
            if(param == NULL)
            {
                printLineW("Usage: create <filename>");
                continue;
            }
            if(createFile(param, File) != ERROR)
            {
                printLineW("File created successfully!");
            }
            else
            {
                printLineW("Failed to create file!");
            }
        }
        else if(!strcmp(cmd, "delete"))
        {
            char* param = strtok(NULL, " ");
            if(param == NULL)
            {
                printLineW("Usage: delete <filename>");
                continue;
            }
            if(deleteFile(param) != ERROR)
            {
                printLineW("File deleted successfully!");
            }
            else
            {
                printLineW("Failed to delete file!");
            }
        }
        else if(!strcmp(cmd, "createDir"))
        {
            char* param = strtok(NULL, " ");
            if(param == NULL)
            {
                printLineW("Usage: createDir <dirname>");
                continue;
            }
            if(createFile(param, Directory) != ERROR)
            {
                printLineW("Directory created successfully!");
            }
            else
            {
                printLineW("Failed to create directory!");
            }
        }
        else if(!strcmp(cmd, "help"))
        {
            printLineW("Available commands:");
            printLineW("ls - list files and directories");
            printLineW("create <filename> - create a new file");
            printLineW("delete <file/dir name> - delete a file/dir");
            printLineW("createDir <dirname> - create a new directory");
            printLineW("read <filename> - read contents of a file");
            printLineW("write <filename> <data> - write data to a file");
            printLineW("repeat <text> - repeat the given text");
            printLineW("exit - exit the shell");
        }
        else
        {
            printLine("Unknown command!", LIGHT_RED);
        }
    }
    kfree(string);
}

void printTitle()
{
    printLine(" ________  ________  ________  ________      ", GREEN);
    printLine("|\\   __  \\|\\   __  \\|\\   __  \\|\\   ____\\     ", GREEN);
    printLine("\\ \\  \\|\\  \\ \\  \\|\\ /\\ \\  \\|\\  \\ \\  \\___|_    ", GREEN);
    printLine(" \\ \\  \\\\\\  \\ \\   __  \\ \\  \\\\  \\ \\_____  \\   ", GREEN);
    printLine("  \\ \\  \\\\\\  \\ \\  \\|\\  \\ \\  \\\\  \\|____|\\  \\  ", GREEN);
    printLine("   \\ \\_______\\ \\_______\\ \\_______\\____\\_\\  \\ ",GREEN);
    printLine("    \\|_______|\\|_______|\\|_______|\\_________\\", GREEN);
    printLine("                                 \\|_________|", GREEN);
    printLine("Version: 0.5", LIGHT_BLUE);
    printLine("Made By: Omer saban and Baraksh", LIGHT_BLUE);
    disableInterrupts();
}

void obos_main()
{
    //basic initalization
    clearScreen();
    disableInterrupts();
    setupIDT();
    printLine("IDT Setup Complete...", LIGHT_BLUE);
    initalizeException();
    printLine("Exception Handling Initialized...", LIGHT_BLUE);
    initializePIC();
    printLine("PIC Initialized...", LIGHT_BLUE);
    loadIDT();
    printLine("IDT Loaded...", LIGHT_BLUE);
    initializeTimer();
    printLine("Timer Initialized...", LIGHT_BLUE);
    // maskAllInterrupts(); //no need to maksk interrupts here
    enableInterrupts();
    clearScreen();
    initializeMemoryManager();
    // printMemoryManagerInfo();
    initializePaging();
    enablePagingNow();
    initKernelHeap();
    initUserHeap();
    clearScreen();
    printTitle(); //this disables interrupts in the os
    loadSuperBlock();
    shell();
    //minimal shutdown
    asm volatile("hlt");
    // enterUserMode((void*)USER_SPACE_START);
    // // unsigned int* ptr = (unsigned int*)0xDEADBEEF;
    // unsigned int value = *ptr; //this will cause a page fault
    while (1);
}
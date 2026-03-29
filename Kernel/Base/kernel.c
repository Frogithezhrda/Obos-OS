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

#define BEEP_SAMPLE_RATE 8000
#define BEEP_FREQ        440
#define BEEP_LEN         (BEEP_SAMPLE_RATE / BEEP_FREQ)

static char beep_buf[BEEP_LEN];

void beepStart(void) {
    int half = BEEP_LEN / 2;
    for (int i = 0; i < BEEP_LEN; i++)
        beep_buf[i] = i < half ? 0xFF : 0x00;

    playSound(beep_buf, BEEP_LEN, BEEP_SAMPLE_RATE);
}
void shutdown()
{
    outw(0x604, 0x2000);
}

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
        else if(!strcmp(cmd, "secret"))
        {
            char* buffer = kmalloc(10);
            printW("password: ");
            keybos(buffer, 10);
            if(!strcmp(buffer, "coco123"))
            {
                printLineW("Its Time For a BAD TIME:)");
                playSound(megalovania, sizeof(megalovania), MEGALOVANIA_SAMPLE_RATE);
                sleep(3000);
                stopSound();
            }
        }
        else if(!strcmp(cmd, "calc"))
        {
            menu();
        }
        else if(!strcmp(cmd, "snake"))
        {
            char* param = strtok(NULL, " ");
            //bad habit sry
            enum Difficulty{EASY = 30, MEDIUM = 20, HARD = 10};
            if(param != NULL)
            {
                if(!strcmp(param, "easy"))
                {
                    setDiff(EASY);
                }
                else if(!strcmp(param, "medium"))
                {
                    setDiff(MEDIUM);
                }
                else if(!strcmp(param, "hard"))
                {
                    setDiff(HARD);
                }
                else
                {
                    printLineW("Usage: snake [easy|medium|hard]");
                    continue;
                }
            }
            runSnakeGame();
        }
        else if(!strcmp(cmd, "edit"))
        {
            char* param = strtok(NULL, " ");
            if(param == NULL)
            {
                printLineW("Usage: edit <filename>");
                continue;
            }
            editFile(param);
        }
        else if(!strcmp(cmd, "exit"))
        {
            printLineW("Exiting...");
            shutdown();
            break;
        }
        else if(!strcmp(cmd, "ls"))
        {
            ls();
        }
        else if(!strcmp(cmd, "cd"))
        {
            char* param = strtok(NULL, "\0");
            if(param == NULL)
            {
                printLineW("Usage: cd <directory>");
                continue;
            }
            if(cd(param) != ERROR)
            {
                printLineW("Changed directory successfully!");
            }
            else
            {
                printLineW("Failed to change directory!");
            }
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
            char* buffer = (char*)kmalloc(256);
            if(readFile(param, buffer, 256) == ERROR)
            {
                printLineW("Failed to read file!");
                kfree(buffer);
                continue;
            }
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
        else if(!strcmp(cmd, "stats"))
        {
            printLine("System Stats: ", LIGHT_GREEN);
            showSystemStats();
            printLine("Ethernet/IP Stats: ", LIGHT_BLUE);
            print("My MAC: ", LIGHT_CYAN);
            printMAC(getMacAddr());
            print("My IP: ", LIGHT_CYAN);
            printIP(MY_IP);
            print(" (Default Qemu IP)", LIGHT_CYAN);

        }
        else if(!strcmp(cmd, "ping"))
        {
            char* param = strtok(NULL, " ");
            if(param == NULL)
            {
                printLineW("Usage: ping <ip>");
                continue;
            }
            icmpSendEchoRequest(splitIP(param));
        }
        else if(!strcmp(cmd, "time"))
        {
            printCurrentTime();
        }
        else if(!strcmp(cmd, "arp"))
        {
            char* param = strtok(NULL, " ");
            if(param == NULL)
            {
                printLineW("Usage: arp <ip>");
                continue;
            }
            arpPrintCache();
            sleep(1000);
            arpRequest(splitIP(param));
            arpPrintCache();
        }
        else if(!strcmp(cmd, "createDir"))
        {
            char* param = strtok(NULL, " ");
            if(param == NULL)
            {
                printLineW("Usage: createDir <dirname>");
                continue;
            }
            if(createDir(param) != ERROR)
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
            printLineW("cd <dirname> - change directory");
            printLineW("read <filename> - read contents of a file");
            printLineW("write <filename> <data> - write data to a file");
            printLineW("repeat <text> - repeat the given text");
            printLineW("edit <filename> - edit a file");
            printLineW("snake <diff>(optional) - play the snake game");
            printLineW("calc - open the calculator");
            printLineW("stats - show system statistics");
            printLineW("arp <ip> - send an arp request");
            printLineW("ping <ip> - just the default ping!");
            printLineW("time - show current time");
            printLineW("exit - exit the os");
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
    printLine("Version: 0.6", LIGHT_BLUE);
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
    initializeNet(); //net
    soundBlasterInit();
    loadSuperBlock();
    generateFinish();
    generateStart();
    //default arp cache entry for the gateway so we can have some sort of network without waiting for an arp request from the gateway
    // arpRequest(QEMU_GATEWAY);
    // arpCacheInsert(QEMU_GATEWAY, arpLookup(QEMU_GATEWAY));
    //simple starting sound for the os, also tests the sound driver and the audio generation
    sleep(100);
    playSound(start, sizeof(start), SAMPLE_RATE);
    sleep(1000);
    playSound(finish, sizeof(finish), SAMPLE_RATE);
    sleep(100);
    stopSound();
    createFile("users.dat", File);
    writeFile("users.dat", "omer&2882598092&526223844\nbarak&3721853714&1533733554", 54);
    
    //add on presentation
    // if(loginMenu() == ERROR)
    // {
    //     printLineW("Failed to login after 3 attempts, shutting down...");
    //     sleep(2000);
    //     shutdown();
    //     return;
    // }
    //username: omer, password: saban1254, salt: 2882598092, hash: 526223844
    //username: barak, password: baraksh123, salt: 3721853714, hash: 1533733554
    shell();

    //minimal shutdown
    // enterUserMode((void*)USER_SPACE_START);
    // // unsigned int* ptr = (unsigned int*)0xDEADBEEF;
    // unsigned int value = *ptr; //this will cause a page fault
    while (1);
}
    #include "paging.h"


    PageDirectory* kernelPageDirectory;
    TSS kernelTSS __attribute__((aligned(4)));

    extern char _kernel_physical_start;
    extern char _kernel_physical_end;


    void initializePaging(void)
    {
        asm volatile("cli"); //turnin off hardware interrupts during setup
        
        unsigned long long pdPhys = allocateFrame(PAGING_STRUCT_START);
        //4 page tables for the first 16MB for more just creating more page tables
        if(pdPhys == ERROR)
        {
            kernelPanic("Error: Unable to allocate frame for Page Directory!");
        }
        kernelPageDirectory = (PageDirectory*)(unsigned int)pdPhys;
        
        memset(kernelPageDirectory, 0, PAGE_SIZE);
        
        for (unsigned int tableNum = 0; tableNum < NUM_PAGE_TABLES_KERNEL; tableNum++)
        {
            unsigned long long ptPhys = allocateFreeFrame();
            if(ptPhys == ERROR)
            {
                kernelPanic("Error: Unable to allocate frame for Page Table!");
            }
            PageTable* pt = (PageTable*)(unsigned int)ptPhys;
            memset(pt, 0, PAGE_SIZE);
            
            unsigned int baseFrameNum = tableNum * PAGE_TABLE_COUNT;

            for (unsigned int i = 0; i < PAGE_TABLE_COUNT; i++)
            {
                pt->entries[i].present = 1;
                pt->entries[i].readWrite = READ_WRITE;
                pt->entries[i].userSupervisor = SUPERVISOR_ONLY;
                pt->entries[i].frameAddress = baseFrameNum + i;
            }
            
            kernelPageDirectory->entries[tableNum].present = 1;
            kernelPageDirectory->entries[tableNum].readWrite = READ_WRITE;
            kernelPageDirectory->entries[tableNum].userSupervisor = SUPERVISOR_ONLY;
            kernelPageDirectory->entries[tableNum].pageTableAddress = ptPhys / PAGE_SIZE;
        }
        /*
        we are using dual mapping here
        mapping the kernel to both low memory and higher half
        so we can access the same address in both modes
        and also higher addresses are protected from user mode
        */
        unsigned int kernelPhysStart = (unsigned int)&_kernel_physical_start;
        unsigned int kernelPhysEnd = (unsigned int)&_kernel_physical_end;
        unsigned int kernelSize = kernelPhysEnd - kernelPhysStart;
        
        mapMemoryRegion(VIRTUAL_VGA_ADDRESS, 
                        PAGE_SIZE, 
                        VGA_PHYSICAL_ADDRESS, 
                        SUPERVISOR_ONLY);
        
        mapMemoryRegion(VIRTUAL_KERNEL_START_ADDRESS,
                        kernelSize,
                        kernelPhysStart,
                        SUPERVISOR_ONLY);
        
        mapMemoryRegion(VIRTUAL_HEAP_START_ADDRESS,
                        HEAP_PHYSICAL_END - HEAP_PHYSICAL_START,
                        HEAP_PHYSICAL_START,
                        SUPERVISOR_ONLY);
        
        mapMemoryRegion(VIRTUAL_STACK_START_ADDRESS,
                        STACK_PHYSICAL_END - STACK_PHYSICAL_START,
                        STACK_PHYSICAL_START,
                        SUPERVISOR_ONLY);
        mapUserPages();


        asm volatile("mov %0, %%cr3" :: "r"(pdPhys) : "memory");
    }


    void enablePagingNow(void)
    {
        unsigned int cr0;
        unsigned int esp;
        asm volatile("mov %%esp, %0" : "=r"(esp));
        // printNumber(esp, WHITE);
        // printLine(" <- ESP before enabling paging", WHITE);
        asm volatile(
            "movl %[stack_top], %%esp\n\t"
            :
            : [stack_top] "r" (VIRTUAL_KERNEL_STACK_TOP)
        );
        
        asm volatile("mov %%esp, %0" : "=r"(esp));
        // printNumber(esp, WHITE);
        // printLine(" <- ESP after setting stack", WHITE);
        
        asm volatile("mov %%cr0, %0" : "=r"(cr0));
        cr0 |= 0x80000000;  //PG bit
        asm volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");

        asm volatile ("sti"); //enabling hardware interrupts after setup
        print("Paging Enabled!\n", GREEN);
        initTSS();
    }

    unsigned long long translateVirtualToPhysical(unsigned long long virtualAddress)
    {
        unsigned int va = (unsigned int)virtualAddress;

        //page directory index by getting bits 22-31
        unsigned int pdIndex = va / PAGE_DIRECTORY_SIZE;
        unsigned int ptIndex = (va / PAGE_SIZE) % PAGE_TABLE_COUNT;
        unsigned int offset = va % PAGE_SIZE;

        if (!kernelPageDirectory->entries[pdIndex].present)
        {
            printLine("PD entry not present", RED);
            return 0;
        }

        PageTable* pt = (PageTable*)(kernelPageDirectory->entries[pdIndex].pageTableAddress * PAGE_SIZE);

        if (!pt->entries[ptIndex].present)
        {
            printLine("PT entry not present", RED);
            return 0;
        }

        return ((unsigned long long)pt->entries[ptIndex].frameAddress * PAGE_SIZE) | offset;
    }



    void mapMemoryRegion(const unsigned long long virtualStart, 
                        const unsigned long long virtualSize, 
                        const unsigned long long physicalStart, 
                        const unsigned int isKernel)
    {
        unsigned long long virtualStartLocal = virtualStart;
        unsigned long long physicalStartLocal = physicalStart;
        unsigned long long virtualEnd = virtualStart + virtualSize;

        while (virtualStartLocal < virtualEnd) 
        {
            mapPage(virtualStartLocal, physicalStartLocal, isKernel);
            virtualStartLocal += PAGE_SIZE;
            physicalStartLocal += PAGE_SIZE;
        }
    }

    PageTable* getOrCreatePageTable(unsigned int virtualAddr, unsigned int isKernel)
    {
        unsigned int pdIndex = virtualAddr >> 22;

        if (!kernelPageDirectory->entries[pdIndex].present)
        {
            unsigned long long ptPhys = allocateFreeFrame();
            if (ptPhys == ERROR)
                kernelPanic("No PT frame");

            PageTable* pt = (PageTable*)(unsigned int)ptPhys;
            memset(pt, 0, PAGE_SIZE);

            kernelPageDirectory->entries[pdIndex].present = 1;
            kernelPageDirectory->entries[pdIndex].readWrite = READ_WRITE;
            kernelPageDirectory->entries[pdIndex].userSupervisor = isKernel ? SUPERVISOR_ONLY : USER_SUPERVISOR;
            kernelPageDirectory->entries[pdIndex].pageTableAddress = ptPhys / PAGE_SIZE;

            return pt;
        }

        if (!isKernel)
            kernelPageDirectory->entries[pdIndex].userSupervisor = USER_SUPERVISOR;

        return (PageTable*)(kernelPageDirectory->entries[pdIndex].pageTableAddress * PAGE_SIZE);
    }



    void mapPage(unsigned int virtualAddr, unsigned int physicalAddr, unsigned int isKernel)
    {
        PageTable* pt = getOrCreatePageTable(virtualAddr, isKernel);
            

        unsigned int ptIndex = (virtualAddr / PAGE_SIZE) % PAGE_TABLE_COUNT;
        
        pt->entries[ptIndex].present = 1;
        pt->entries[ptIndex].readWrite = READ_WRITE;
        pt->entries[ptIndex].userSupervisor = isKernel ? SUPERVISOR_ONLY : USER_SUPERVISOR;
        pt->entries[ptIndex].frameAddress = physicalAddr / PAGE_SIZE;
    }

    void mapUserPages()
    {
        unsigned long addr = USER_SPACE_START;
        
        print("Setting up user space page tables...\n", GREEN);
        
        int count = 0;
        while (addr < USER_SPACE_END)
        {
            unsigned int pdIndex = addr >> 22;
            
            if (!kernelPageDirectory->entries[pdIndex].present)
            {
                unsigned long long ptPhys = allocateFreeFrame();
                if (ptPhys == ERROR)
                    kernelPanic("No frame for page table!");
                
                PageTable* pt = (PageTable*)(unsigned int)ptPhys;
                memset(pt, 0, PAGE_SIZE);
                
                kernelPageDirectory->entries[pdIndex].present = 1;
                kernelPageDirectory->entries[pdIndex].readWrite = READ_WRITE;
                kernelPageDirectory->entries[pdIndex].userSupervisor = USER_SUPERVISOR;
                kernelPageDirectory->entries[pdIndex].pageTableAddress = ptPhys / PAGE_SIZE;
                
                count++;
            }
            
            addr += PAGE_DIRECTORY_SIZE;
        }
        
        unsigned long long codePhys = allocateFreeFrame();
        if (codePhys == ERROR) kernelPanic("No code frame!");
        memset((void*)(unsigned int)codePhys, 0, PAGE_SIZE);
        mapPage(USER_SPACE_START, (unsigned int)codePhys, 0);
        
        unsigned long long stackPhys = allocateFreeFrame(); 
        if (stackPhys == ERROR) kernelPanic("No stack frame!");
        memset((void*)(unsigned int)stackPhys, 0, PAGE_SIZE);
        mapPage(USER_STACK_TOP - PAGE_SIZE, (unsigned int)stackPhys, 0);

        //a small user program in order to check for demand paging
        unsigned char* code = (unsigned char*)codePhys;
        int i = 0;
        //this is a small program that keeps everything alive
        // code[i++] = 0xEB;
        // code[i++] = 0xFE;
        // a small program that prints Hello from user mode!
//         // mov eax, 0
        // code[i++] = 0xB8;
        // code[i++] = 0x00;
        // code[i++] = 0x00;
        // code[i++] = 0x00;
        // code[i++] = 0x00;
        // // mov ebx, USER_SPACE_START + 0x50
        // code[i++] = 0xBB;
        // addr = USER_SPACE_START + 0x50;
        // code[i++] = (addr >> 0) & 0xFF;
        // code[i++] = (addr >> 8) & 0xFF;
        // code[i++] = (addr >> 16) & 0xFF;
        // code[i++] = (addr >> 24) & 0xFF;

        // // // int 0x80
        // code[i++] = 0xCD;
        // code[i++] = 0x80;

        // // jmp $
        // code[i++] = 0xEB;
        // code[i++] = 0xFE;

        // code[0x50] = 'H';
        // code[0x51] = 'e';
        // code[0x52] = 'l';
        // code[0x53] = 'l';
        // code[0x54] = 'o';
        // code[0x55] = ' ';
        // code[0x56] = 'f';
        // code[0x57] = 'r';
        // code[0x58] = 'o';
        // code[0x59] = 'm';
        // code[0x5A] = ' ';
        // code[0x5B] = 'u';
        // code[0x5C] = 's';
        // code[0x5D] = 'e';
        // code[0x5E] = 'r';
        // code[0x5F] = ' ';
        // code[0x60] = 'm';
        // code[0x61] = 'o';
        // code[0x62] = 'd';
        // code[0x63] = 'e';
        // code[0x64] = '!';
        // code[0x65] = 0x00;
        // int i = 0;
        // unsigned int addr;

        // Test 1: Print without color (SYSCALL_PRINT with arg2=0)
        /* mov eax, 0 */
        code[i++] = 0xB8;
        code[i++] = 0x00;
        code[i++] = 0x00;
        code[i++] = 0x00;
        code[i++] = 0x00;
        /* mov ebx, USER_SPACE_START + 0x50 (string pointer) */
        code[i++] = 0xBB;
        addr = USER_SPACE_START + 0x50;
        code[i++] = (addr >> 0) & 0xFF;
        code[i++] = (addr >> 8) & 0xFF;
        code[i++] = (addr >> 16) & 0xFF;
        code[i++] = (addr >> 24) & 0xFF;
        /* mov ecx, 0 (no color) */
        code[i++] = 0xB9;
        code[i++] = 0x00;
        code[i++] = 0x00;
        code[i++] = 0x00;
        code[i++] = 0x00;
        /* int 0x80 */
        code[i++] = 0xCD;
        code[i++] = 0x80;

        // Test 2: Print with color (SYSCALL_PRINT with arg2=color)
        /* mov eax, 0 */
        code[i++] = 0xB8;
        code[i++] = 0x00;
        code[i++] = 0x00;
        code[i++] = 0x00;
        code[i++] = 0x00;
        /* mov ebx, USER_SPACE_START + 0x70 (colored string pointer) */
        code[i++] = 0xBB;
        addr = USER_SPACE_START + 0x70;
        code[i++] = (addr >> 0) & 0xFF;
        code[i++] = (addr >> 8) & 0xFF;
        code[i++] = (addr >> 16) & 0xFF;
        code[i++] = (addr >> 24) & 0xFF;
        /* mov ecx, GREEN (assuming GREEN = 10) */
        code[i++] = 0xB9;
        code[i++] = 0x0A;  // GREEN color value
        code[i++] = 0x00;
        code[i++] = 0x00;
        code[i++] = 0x00;
        /* int 0x80 */
        code[i++] = 0xCD;
        code[i++] = 0x80;

        // Test 3: Get ticks (SYSCALL_GET_TICKS)
        /* mov eax, 1 */
        code[i++] = 0xB8;
        code[i++] = 0x01;
        code[i++] = 0x00;
        code[i++] = 0x00;
        code[i++] = 0x00;
        /* mov ebx, USER_SPACE_START + 0x100 (location to store ticks) */
        code[i++] = 0xBB;
        addr = USER_SPACE_START + 0x100;
        code[i++] = (addr >> 0) & 0xFF;
        code[i++] = (addr >> 8) & 0xFF;
        code[i++] = (addr >> 16) & 0xFF;
        code[i++] = (addr >> 24) & 0xFF;
        /* int 0x80 */
        code[i++] = 0xCD;
        code[i++] = 0x80;

        // Test 4: Sleep (SYSCALL_SLEEP = 7)
        /* mov eax, 7 */
        code[i++] = 0xB8;
        code[i++] = 0x07;
        code[i++] = 0x00;
        code[i++] = 0x00;
        code[i++] = 0x00;
        /* mov ebx, 1000 (sleep duration in ms) */
        code[i++] = 0xBB;
        code[i++] = 0xE8;
        code[i++] = 0x05;
        code[i++] = 0x00;
        code[i++] = 0x00;
        /* int 0x80 */
        code[i++] = 0xCD;
        code[i++] = 0x80;

        /* jmp $ (infinite loop) */
        code[i++] = 0xEB;
        code[i++] = 0xFE;

        // String data at offset 0x50
        code[0x50] = 'H';
        code[0x51] = 'e';
        code[0x52] = 'l';
        code[0x53] = 'l';
        code[0x54] = 'o';
        code[0x55] = ' ';
        code[0x56] = 'w';
        code[0x57] = 'o';
        code[0x58] = 'r';
        code[0x59] = 'l';
        code[0x5A] = 'd';
        code[0x5B] = '!';
        code[0x5C] = 0x00;

        // String data at offset 0x70 (for colored print)
        code[0x70] = 'C';
        code[0x71] = 'o';
        code[0x72] = 'l';
        code[0x73] = 'o';
        code[0x74] = 'r';
        code[0x75] = 'e';
        code[0x76] = 'd';
        code[0x77] = ' ';
        code[0x78] = 't';
        code[0x79] = 'e';
        code[0x7A] = 'x';
        code[0x7B] = 't';
        code[0x7C] = '!';
        code[0x7D] = 0x00;

        // Reserve space at 0x100 for ticks value (will be written by syscall)
        print("User space ready\n", CYAN);
    }

    void initTSS()
    {
        memset(&kernelTSS, 0, sizeof(TSS));
        
        kernelTSS.ss0 = 0x10;
        kernelTSS.esp0 = VIRTUAL_KERNEL_STACK_TOP;
        kernelTSS.cs = 0x08;
        kernelTSS.ss = 0x10;
        kernelTSS.ds = 0x10;
        kernelTSS.es = 0x10;
        kernelTSS.fs = 0x10;
        kernelTSS.gs = 0x10;
        
        kernelTSS.iomap_base = sizeof(TSS);
        
        unsigned int tssAddress = (unsigned int)&kernelTSS;
        
        GDTR gdtr;
        
        asm volatile("sgdt %0" : "=m"(gdtr));
        
        unsigned char* tssDesc = (unsigned char*)(gdtr.base + 0x28);
        
        tssDesc[2] = (tssAddress) & 0xFF;
        tssDesc[3] = (tssAddress / 256) & 0xFF;
        tssDesc[4] = (tssAddress / 65536) & 0xFF;
        tssDesc[7] = (tssAddress / 16777216) & 0xFF;
        
        asm volatile("ltr %0" : : "r"((unsigned short)TSS_SELECTOR));

    }


    void enterUserMode(void* userEntryPoint)
    {
        unsigned int entry = (unsigned int)userEntryPoint;
        unsigned int stack = USER_STACK_TOP;
    
        print("Jumping to user mode: EIP=0x", GREEN);
        printNumber(entry, GREEN);
        print(", ESP=0x", GREEN);
        printNumber(stack, GREEN);
        printLine("", GREEN);
        printNumber(translateVirtualToPhysical(USER_SPACE_START), GREEN);
        printLine("", WHITE);
        printNumber(translateVirtualToPhysical(USER_STACK_TOP - 4), GREEN);
        printLine("", WHITE);
        asm volatile(
            "mov %0, %%ebx\n"        //save entry in ebx
            "mov %1, %%ecx\n"        //save stack in ecx
            
            "cli\n"
            
            "pushl $0x23\n"          // SS
            "pushl %%ecx\n"          // ESP
            
            "pushfl\n"
            "popl %%eax\n"
            "orl  $0x200, %%eax\n"
            "pushl %%eax\n"          // EFLAGS
            
            "pushl $0x1B\n"          // CS
            "pushl %%ebx\n"          // EIP
            
            "iret\n"
            :
            : "r"(entry), "r"(stack)
            : "eax", "ebx", "ecx"
        );
    }
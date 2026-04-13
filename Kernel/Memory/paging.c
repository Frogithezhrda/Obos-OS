#include "paging.h"


PageDirectory* kernelPageDirectory;
TSS kernelTSS __attribute__((aligned(4)));
InterruptFrame kernelFrame;

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

    unsigned int testAddr = 0x1090E8;
    unsigned int pdIdx = testAddr >> 22;
    unsigned int ptIdx = (testAddr >> 12) & 0x3FF;

    printW("Testing addr 0x1090E8: PD["); printNumberW(pdIdx);
    printW("] PT["); printNumberW(ptIdx); printW("]\n");
    printW("PD entry present: "); 
    printNumberW(kernelPageDirectory->entries[pdIdx].present); printLineW("");

    if(kernelPageDirectory->entries[pdIdx].present)
    {
        PageTable* pt = (PageTable*)(kernelPageDirectory->entries[pdIdx].pageTableAddress * PAGE_SIZE);
        printW("PT entry present: "); printNumberW(pt->entries[ptIdx].present); printLineW("");
        printW("Frame: 0x"); printHexW(pt->entries[ptIdx].frameAddress * PAGE_SIZE); printLineW("");
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
    
    mapMemoryRegion(VIRTUAL_KERNEL_START_ADDRESS,
                    VIRTUAL_KERNEL_END_ADDRESS - VIRTUAL_KERNEL_START_ADDRESS,
                    KERNEL_PHYSICAL_START,
                    SUPERVISOR_ONLY);
    
    mapMemoryRegion(VIRTUAL_HEAP_START_ADDRESS,
                    HEAP_PHYSICAL_END - HEAP_PHYSICAL_START,
                    HEAP_PHYSICAL_START,
                    SUPERVISOR_ONLY);
    
    mapMemoryRegion(VIRTUAL_STACK_START_ADDRESS,
                    STACK_PHYSICAL_END - STACK_PHYSICAL_START,
                    STACK_PHYSICAL_START,
                    SUPERVISOR_ONLY);

    mapMemoryRegion(VBE_FRAMEBUFFER_START,
                    VBE_FRAMEBUFFER_END - VBE_FRAMEBUFFER_START,
                    VBE_FRAMEBUFFER_START,
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
        : [stack_top] "r" (STACK_PHYSICAL_END)
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
    
    // int count = 0;
    // while (addr < USER_SPACE_END)
    // {
    //     unsigned int pdIndex = addr >> 22;
        
    //     if (!kernelPageDirectory->entries[pdIndex].present)
    //     {
    //         unsigned long long ptPhys = allocateFreeFrame();
    //         if (ptPhys == ERROR)
    //             kernelPanic("No frame for page table!");
            
    //         PageTable* pt = (PageTable*)(unsigned int)ptPhys;
    //         memset(pt, 0, PAGE_SIZE);
            
    //         kernelPageDirectory->entries[pdIndex].present = 1;
    //         kernelPageDirectory->entries[pdIndex].readWrite = READ_WRITE;
    //         kernelPageDirectory->entries[pdIndex].userSupervisor = USER_SUPERVISOR;
    //         kernelPageDirectory->entries[pdIndex].pageTableAddress = ptPhys / PAGE_SIZE;
            
    //         count++;
    //     }
        
    //     addr += PAGE_DIRECTORY_SIZE;
    // }
    
    unsigned int pages = ((PAGE_SIZE * 1) / PAGE_SIZE) + 1;

    for (unsigned int i = 0; i < pages; i++)
    {
        unsigned int phys = allocateFreeFrame();
        if (phys == ERROR) kernelPanic("No frame for user code!");

        memset((void*)phys, 0, PAGE_SIZE);

        mapPage(USER_SPACE_START + i * PAGE_SIZE, phys, 0);
    }

    unsigned long long stackPhys = allocateFreeFrame(); 
    if (stackPhys == ERROR) kernelPanic("No stack frame!");
    memset((void*)(unsigned int)stackPhys, 0, PAGE_SIZE);
    mapPage(USER_STACK_TOP - PAGE_SIZE, (unsigned int)stackPhys, 0);
    //a small user program in order to check for demand paging
    unsigned char* code = (unsigned char*)USER_SPACE_START;
    int i = 0;
    //this is a small program that keeps everything alive
    code[i++] = 0xEB;
    code[i++] = 0xFE;

    // a small program that prints Hello from user mode!
    // unsigned int addr;
    // Reserve space at 0x100 for ticks value (will be written by syscall)
    print("User space ready\n", CYAN);
}

void initTSS()
{
    memset(&kernelTSS, 0, sizeof(TSS));
    
    kernelTSS.ss0 = 0x10;
    kernelTSS.esp0 = STACK_PHYSICAL_END;
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
    printW("kernelTSS phys addr: "); 
    printHexW((unsigned int)&kernelTSS); 
    printLineW("");
    asm volatile("ltr %0" : : "r"((unsigned short)TSS_SELECTOR));

}

void enterUserMode(void* userEntryPoint, void* kernelPoint)
{
    unsigned int entry = (unsigned int)userEntryPoint;
    unsigned int stack = USER_STACK_TOP;
    // loadUserProgram();
    asm volatile(
        "mov %0, %%ebx\n"
        "mov %1, %%ecx\n"

        "cli\n"

        "pushl $0x23\n"
        "pushl %%ecx\n"

        "pushfl\n"
        "popl %%eax\n"
        "orl  $0x200, %%eax\n"
        "pushl %%eax\n"

        "pushl $0x1B\n"
        "pushl %%ebx\n"

        "sti\n"

        "iret\n"
        :
        : "r"(entry), "r"(stack)
        : "eax", "ebx", "ecx"
    );
}
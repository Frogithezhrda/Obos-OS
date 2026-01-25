#include "paging.h"

PageDirectory* kernelPageDirectory;



void initializePaging(void)
{
    asm volatile("cli"); //turnin off hardware interrupts during setup
    
    unsigned long long pdPhys = allocateFreeFrame();
    //4 page tables for the first 16MB for more just creating more page tables
    
    kernelPageDirectory = (PageDirectory*)(unsigned int)pdPhys;
    
    memset(kernelPageDirectory, 0, PAGE_SIZE);
    
    for (unsigned int tableNum = 0; tableNum < NUM_PAGE_TABLES_KERNEL; tableNum++)
    {
        unsigned long long ptPhys = allocateFreeFrame();
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
        
    asm volatile("mov %0, %%cr3" :: "r"(pdPhys) : "memory");
}

void enablePagingNow(void)
{
    unsigned int cr0;
    unsigned int esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    printNumber(esp, WHITE);
    printLine(" <- ESP before enabling paging", WHITE);
    asm volatile(
        "movl %[stack_top], %%esp\n\t"
        :
        : [stack_top] "r" (KERNEL_STACK_TOP)
    );
    
    asm volatile("mov %%esp, %0" : "=r"(esp));
    printNumber(esp, WHITE);
    printLine(" <- ESP after setting stack", WHITE);
    
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  //PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");

    asm volatile ("sti"); //enabling hardware interrupts after setup
    print("Paging Enabled!\n", GREEN);
}

unsigned long long translateVirtualToPhysical(unsigned long long virtualAddress)
{
    unsigned int va = (unsigned int)virtualAddress;

    //page directory index by getting bits 22-31
    unsigned int pdIndex = (va >> 22) & (PAGE_TABLE_COUNT - 1);
    //page table index by getting bits 12-21
    unsigned int ptIndex = (va / PAGE_SIZE) & (PAGE_TABLE_COUNT - 1);
    //offset within the page by getting bits 0-11
    unsigned int offset  = va & 0xFFF;

    if (!kernelPageDirectory->entries[pdIndex].present)
    {
        printLine("PD entry not present", RED);
        return 0;
    }

    PageTable* pt = (PageTable*)
        (kernelPageDirectory->entries[pdIndex].pageTableAddress * PAGE_SIZE);

    if (!pt->entries[ptIndex].present)
    {
        printLine("PT entry not present", RED);
        return 0;
    }

    return ((unsigned long long)pt->entries[ptIndex].frameAddress * PAGE_SIZE) | offset;
}

void pageFaultHandler(unsigned int errorCode)
{
    (void)errorCode;
    printLine("Page Fault Detected!", RED);
    asm volatile("cli");
    while (1);   // halt safely
}

void mapMemoryRegion(PageTable* pageTable, 
                     const unsigned long long virtualStart, 
                     const unsigned long long virtualEnd, 
                     const unsigned long long physicalStart, 
                     const unsigned int isKernel)
{

    unsigned long long va = virtualStart;
    unsigned long long pa = physicalStart;

    while (va < virtualEnd) 
    {
        unsigned int ptIndex = (va / PAGE_SIZE) & (PAGE_TABLE_COUNT - 1);

        pageTable->entries[ptIndex].present = 1;
        pageTable->entries[ptIndex].readWrite = READ_WRITE;
        pageTable->entries[ptIndex].userSupervisor = isKernel ? SUPERVISOR_ONLY : USER_SUPERVISOR;
        pageTable->entries[ptIndex].frameAddress = pa / PAGE_SIZE;

        va += PAGE_SIZE;
        pa += PAGE_SIZE;
    }
}
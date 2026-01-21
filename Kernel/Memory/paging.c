#include "paging.h"

PageTable* kernelPageTable;

PageDirectory* kernelPageDirectory;

/*
TODO Create a Page Directory For CR3 Register
and point it to the kernel page table
and handle multiple page tables
verify that paging is working correctly

*/
void initializePaging(void)
{
    unsigned long long pageDirectoryAddress = allocateFreeFrame();


    if(pageDirectoryAddress == ERROR) 
    {
        //allocation failed
        //TODO throw kernel panic exception
        printLine("Error: Out of memory while initializing kernel page directory!", RED);
        return;
    }

    kernelPageDirectory = (PageDirectory*)pageDirectoryAddress;
    
    for(unsigned int i = 0; i < PAGE_TABLE_COUNT; i++)
    {
        kernelPageTable->entries[i].isPresent = 1;
        kernelPageTable->entries[i].permissionBits = READ_WRITE;
        kernelPageTable->entries[i].frameAddress = (i * PAGE_SIZE);
        kernelPageTable->entries[i].accessedBit = 0;
        kernelPageTable->entries[i].dirtyBit = 0;
        kernelPageTable->entries[i].executeBit = 0;
        kernelPageTable->entries[i].kernelBit = 0;
    }

    enablePaging();
}

void initializeKernelPageTable(void)
{
    unsigned long long pageTableAddress = allocateFreeFrame();

    if(pageTableAddress == ERROR) 
    {
        //allocation failed
        //TODO throw kernel panic exception
        printLine("Error: Out of memory while initializing kernel page table!", RED);
        return;
    }

    kernelPageTable = (PageTable*)pageTableAddress; //placing the kernel page table at 1MB

    mapMemoryRegion(kernelPageTable, 
                    KERNEL_START_ADDRESS, 
                    KERNEL_END_ADDRESS, 
                    KERNEL_PHYSICAL_START_ADDRESS, 
                    1, //kernel executable
                    1); //kernel memorr 

    // Map heap memory: 0x104000 - 0x204000 (1 MB)
    mapMemoryRegion(kernelPageTable, 
                    HEAP_START_ADDRESS, 
                    HEAP_END_ADDRESS, 
                    HEAP_PHYSICAL_START_ADDRESS, 
                    0, //heap not exec
                    1); //kernel memory

    mapMemoryRegion(kernelPageTable, 
                    STACK_END_ADDRESS, //switching cause the stack grows downwards
                    STACK_START_ADDRESS, 
                    STACK_PHYSICAL_START_ADDRESS, 
                    0, //stack not exec
                    1); //kernel memory
}


/*
Page - 4KB
Frame - 4KB
but we want to get an address so we take te frame number 
and multiply it by 4KB to get the starting address of the frame
after that we add the offset to get the exact physical address
*/
unsigned long long translateVirtualToPhysical(unsigned long long virtualAddress)
{
    unsigned int pageIndex = virtualAddress / PAGE_SIZE;
    unsigned long long physicalFrame = 0;
    if (!kernelPageTable->entries[pageIndex].isPresent) 
    {
        printLine("Error: Page not present!", RED);

        //TODO throw page fault exception
        return 0;
    }
    physicalFrame = kernelPageTable->entries[pageIndex].frameAddress * PAGE_SIZE;
    unsigned long long offset = virtualAddress % PAGE_SIZE;

    return physicalFrame + offset;
}

void enablePaging(void)
{
    unsigned int cr0;
    //reading cr0 register
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; //setting teh cr0 to true using OR gate
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

void mapMemoryRegion(PageTable* pageTable, 
                     const unsigned long long virtualStart, 
                     const unsigned long long virtualEnd, 
                     const unsigned long long physicalStart, 
                     const unsigned int isExecutable, 
                     const unsigned int isKernel)
{
    for (unsigned long long address = virtualStart, physicalAddress = physicalStart; 
         address < virtualEnd; 
         address += PAGE_SIZE, physicalAddress += PAGE_SIZE) 
    {
        unsigned int index = address / PAGE_SIZE;
        pageTable->entries[index].isPresent = 1;
        pageTable->entries[index].permissionBits = READ_WRITE;
        pageTable->entries[index].frameAddress = physicalAddress / PAGE_SIZE;
        pageTable->entries[index].accessedBit = 0;
        pageTable->entries[index].dirtyBit = 0;
        pageTable->entries[index].executeBit = isExecutable;
        pageTable->entries[index].kernelBit = isKernel;
    }
}
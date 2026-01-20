#ifndef PAGING_H
#define PAGING_H

#include "memoryMap.h"

#define PAGE_SIZE 4096 //4KB
#define PAGE_TABLE_COUNT 512 //each page table has 512 entries

//THOSE ARE VIRTUAL ADDRESSES
#define KERNEL_START_ADDRESS 0x000000
#define KERNEL_END_ADDRESS 0x004000

#define HEAP_START_ADDRESS 0x004000 
#define HEAP_END_ADDRESS 0x100000

#define STACK_END_ADDRESS 0x104000 
#define STACK_START_ADDRESS 0x108000

//PHYSICAL ADDRESSES
#define KERNEL_PHYSICAL_START_ADDRESS 0x100000
// #define KERNEL_PHYSICAL_END_ADDRESS 0x104000

#define HEAP_PHYSICAL_START_ADDRESS 0x104000 
// #define HEAP_PHYSICAL_END_ADDRESS 0x200000

// #define STACK_PHYSICAL_END_ADDRESS 0x200000 
#define STACK_PHYSICAL_START_ADDRESS 0x1FFC00


enum PermissionBits
{
    READ_WRITE = 0b10,
    READ_ONLY = 0b01,
    USER_SUPERVISOR = 0b100,
    SUPERVISOR_ONLY = 0b000
};

typedef struct PageTableEntry
{
    int accessedBit;
    int kernelBit;
    int dirtyBit;
    int isPresent;
    int permissionBits; //read/write, user/supervisor
    int executeBit;
    unsigned long long frameAddress;
} PageTableEntry;

//each page table has 512 entries
//512 * 4kb = 2MB per page table
//if we will need more pages then we will use a page directory to point to multiple page tables
typedef struct PageTable
{
    PageTableEntry entries[PAGE_TABLE_COUNT];
} PageTable;

//currently not needed... but will be used in future for multiple page tables
typedef struct PageDirectory
{
    int accessedBit;
    int kernelBit;
    int dirtyBit;
    int isPresent;
    int permissionBits; //read/write, user/supervisor
    int executeBit;
    PageTable* pageTableAddress;
} PageDirectory;


void enablePaging(void);
void initializePaging(void);
void initializeKernelPageTable(void);
unsigned long long translateVirtualToPhysical(unsigned long long virtualAddress);
void mapMemoryRegion(PageTable* pageTable, 
                     const unsigned long long virtualStart, 
                     const unsigned long long virtualEnd, 
                     const unsigned long long physicalStart, 
                     const unsigned int isExecutable, 
                     const unsigned int isKernel);
#endif  
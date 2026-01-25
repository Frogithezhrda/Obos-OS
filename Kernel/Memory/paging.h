

#ifndef PAGING_H
#define PAGING_H

#include "memoryMap.h"
#include "../SystemLib/obosMemory.h"

#define PAGE_SIZE 4096 //4KB
#define PAGE_TABLE_COUNT 1024 //each page table has 512 entries
#define MAPPED_MEMORY_MB_KERNEL 64
#define NUM_PAGE_TABLES_KERNEL (MAPPED_MEMORY_MB_KERNEL / 4) 

//THOSE ARE VIRTUAL ADDRESSES
#define KERNEL_START_ADDRESS 0x000000
#define KERNEL_END_ADDRESS 0x008000

#define HEAP_START_ADDRESS 0x008000 
#define HEAP_END_ADDRESS 0x100000

#define STACK_END_ADDRESS 0x104000 
#define STACK_START_ADDRESS 0x108000

#define KERNEL_STACK_TOP 0x200000

//PHYSICAL ADDRESSES
#define KERNEL_PHYSICAL_START_ADDRESS 0x100000
#define KERNEL_PHYSICAL_END_ADDRESS 0x108000

#define HEAP_PHYSICAL_START_ADDRESS 0x108000 
#define HEAP_PHYSICAL_END_ADDRESS 0x200000

#define STACK_PHYSICAL_END_ADDRESS 0x200000
#define STACK_PHYSICAL_START_ADDRESS 0x1FFC00


enum PermissionBits
{
    READ_WRITE = 1, // currently changed it did a bug
    READ_ONLY = 0,
    USER_SUPERVISOR = 1,
    SUPERVISOR_ONLY = 0
};

typedef struct PageTableEntry
{
    unsigned int present        : 1; 
    unsigned int readWrite      : 1; 
    unsigned int userSupervisor : 1; 
    unsigned int writeThrough   : 1;
    unsigned int cacheDisable   : 1;
    unsigned int accessed       : 1;
    unsigned int dirty          : 1;
    unsigned int pageSize       : 1;
    unsigned int global         : 1;
    unsigned int available      : 3;
    unsigned int frameAddress   : 20; 
} __attribute__((packed)) PageTableEntry;

//each page table has 512 entries
//512 * 4kb = 2MB per page table
//if we will need more pages then we will use a page directory to point to multiple page tables
typedef struct PageTable
{
    PageTableEntry entries[PAGE_TABLE_COUNT];
} __attribute__((aligned(PAGE_SIZE))) PageTable;

typedef struct PageDirectoryEntry
{
    unsigned int present        : 1;
    unsigned int readWrite      : 1;
    unsigned int userSupervisor : 1;
    unsigned int writeThrough   : 1;
    unsigned int cacheDisable   : 1;
    unsigned int accessed       : 1;
    unsigned int reserved       : 1;
    unsigned int pageSize       : 1;
    unsigned int global         : 1;
    unsigned int available      : 3;
    unsigned int pageTableAddress : 20;
} __attribute__((packed)) PageDirectoryEntry;

typedef struct PageDirectory
{
    PageDirectoryEntry entries[PAGE_TABLE_COUNT];
} __attribute__((aligned(PAGE_SIZE))) PageDirectory;


void pageFaultHandler(unsigned int errorCode);
void enablePaging(void);
void initializePaging(void);
void initializeKernelPageTable(void);
unsigned long long translateVirtualToPhysical(unsigned long long virtualAddress);
void mapMemoryRegion(PageTable* pageTable, 
                     const unsigned long long virtualStart, 
                     const unsigned long long virtualEnd, 
                     const unsigned long long physicalStart, 
                     const unsigned int isKernel);
#endif  
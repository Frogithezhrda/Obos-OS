

#ifndef PAGING_H
#define PAGING_H

#include "memoryMap.h"
#include "../SystemLib/obosMemory.h"

#define PAGE_SIZE 4096 //4KB
#define PAGE_TABLE_COUNT 1024 //each page table has 1024 entries
#define PAGE_DIRECTORY_SIZE  (4 * 1024 * 1024)
#define MAPPED_MEMORY_MB_KERNEL 64
#define NUM_PAGE_TABLES_KERNEL (MAPPED_MEMORY_MB_KERNEL / 4) 

#define KERNEL_VIRTUAL_BASE         0xC0000000

#define KERNEL_START_ADDRESS        0xC0100000
#define KERNEL_END_ADDRESS          0xC0108000

#define VGA_VIRTUAL_ADDRESS         0xC00B8000

#define HEAP_START_ADDRESS          0xC0108000
#define HEAP_END_ADDRESS            0xC0200000

#define STACK_START_ADDRESS         0xC01FFC00
#define STACK_END_ADDRESS           0xC0200000
#define KERNEL_STACK_TOP            0xC0200000



#define KERNEL_PHYSICAL_START       0x00100000 
#define KERNEL_PHYSICAL_END         0x00108000

#define VGA_PHYSICAL_ADDRESS        0x000B8000

#define HEAP_PHYSICAL_START         0x00108000
#define HEAP_PHYSICAL_END           0x00200000

#define STACK_PHYSICAL_START        0x001FFC00
#define STACK_PHYSICAL_END          0x00200000


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
void mapMemoryRegion(const unsigned long long virtualStart, 
                     const unsigned long long virtualSize, 
                     const unsigned long long physicalStart, 
                     const unsigned int isKernel);

void mapPage(unsigned int virtualAddr, unsigned int physicalAddr, unsigned int isKernel);
PageTable* getOrCreatePageTable(unsigned int virtualAddr);
#endif  
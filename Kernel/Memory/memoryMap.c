#include "memoryMap.h"

static MemoryManager memoryManager;
static MemoryMapEntry* memoryMap = (MemoryMapEntry*)MEMORY_MAP_ADDRESS;    


void initializeMemoryManager()
{
    unsigned short entryCount = *((unsigned short*)MEMORY_MAP_ENTTRY_COUNT_ADDRESS);
    
    printNumber(entryCount, WHITE);
    if(!entryCount) printLine("no Memory map entries found..", WHITE);
    parseMemoryMap(entryCount, memoryMap);
    reserveKernelRegions();
    printEntries(entryCount, memoryMap);    
}

void printEntries()
{
    for (unsigned int i = 0; i < memoryManager.regionCount; i++) 
    {
        print("Memory Region: Base = ", WHITE);
        printNumber((unsigned int)memoryManager.regions[i].start, WHITE);
        print(", Length = ", WHITE);
        printNumber((unsigned int)(memoryManager.regions[i].end - memoryManager.regions[i].start), WHITE);
        print(", Type = ", WHITE);
        printNumber(memoryManager.regions[i].type, WHITE);
        print("\n", WHITE);
    }
}

void printMemoryManagerInfo()
{
    unsigned int usableMB = (unsigned int)(memoryManager.totalUsable / (MB));
    unsigned int reservedMB = (unsigned int)(memoryManager.totalReserved / (MB));
    print("Total Usable Memory: ", WHITE);
    printNumber(usableMB, WHITE);
    printLine(" MB", WHITE);
    
    print("Total Reserved Memory: ", WHITE);
    printNumber(reservedMB, WHITE);
    printLine(" MB", WHITE);
}


void parseMemoryMap(const unsigned short entryCount, MemoryMapEntry* memoryMap)
{
    memoryManager.regionCount = 0;
    memoryManager.totalReserved = 0;
    memoryManager.totalUsable = 0;
    for (unsigned int i = 0; i < entryCount; i++) 
    {


        if (memoryMap[i].length == 0 || memoryMap[i].type > MEMORY_TYPE_BAD_MEMORY) 
        {
            continue; 
        }
        MemoryRegion region;
        region.start = memoryMap[i].base;
        region.end = memoryMap[i].base + memoryMap[i].length;
        region.type = memoryMap[i].type;
        region.isFree = (memoryMap[i].type == MEMORY_TYPE_AVAILABLE) ? 1 : 0;


        if (memoryManager.regionCount < REGION_COUNT) 
        {
            memoryManager.regions[memoryManager.regionCount++] = region;
        }
        else
        {
            printLine("Warning: Memory region count exceeded!", RED);
            continue;
        }

        //checking for each memory region type
        if (memoryMap[i].type == MEMORY_TYPE_AVAILABLE) 
        {
            memoryManager.totalUsable += memoryMap[i].length;
        } 
        else
        {
            printLine("Reserved Memory Region Found", RED);
            printNumber((unsigned int)memoryManager.totalReserved, WHITE);
            memoryManager.totalReserved += memoryMap[i].length;
        }
        //marking not free memory
        if(memoryMap[i].type != MEMORY_TYPE_AVAILABLE) 
        {
            region.isFree = 0;
        }
    }
}

/*
0x100000 - 0x104000 (Kernel Size 16384 bytes)
0x200000 - 0x1FFC00 (Stack Size 16KB)
0x104000 - 0x204000 (Heap Size 1MB)
0x100000
Kernel ->

0x104000
Heap ->



<- Stack
0x200000
*/
void reserveKernelRegions(void)
{
    MemoryRegion region;

    region.start = 0x100000; //1MB
    region.end = 0x104000;   //1MB + Kernel Size (16384 bytes)
    region.type = MEMORY_TYPE_RESERVED;
    region.isFree = 0;
    memoryManager.regions[memoryManager.regionCount++] = region;
    //IMPORTANT NOTE: Stack grows downwards
    //so the stack region is from 0x1FFC00 to 0x200000 - but it starts at 0x200000 and grows downwards
    region.start = 0x1FFC00; //2MB
    region.end = 0x200000;   //2MB - Stack Size (16KB)
    region.type = MEMORY_TYPE_RESERVED;
    region.isFree = 0;
    memoryManager.regions[memoryManager.regionCount++] = region;
    region.start = 0x104000; //1MB + Kernel Size
    region.end = 0x200000;   //1MB + Kernel Size + Heap Size (1MB)
    region.type = MEMORY_TYPE_RESERVED;
    region.isFree = 0;
    memoryManager.regions[memoryManager.regionCount++] = region;
    memoryManager.totalReserved += (0x200000 - 0x100000); //Kernel Size + Stack Size + Heap Size

}
#include "memoryMap.h"

static MemoryManager memoryManager;
static MemoryMapEntry* memoryMap = (MemoryMapEntry*)MEMORY_MAP_ADDRESS;    


void initializeMemoryManager()
{
    unsigned short entryCount = *((unsigned short*)MEMORY_MAP_ENTTRY_COUNT_ADDRESS);
    
    printNumber(entryCount, WHITE);
    if(!entryCount) printLine("no Memory map entries found..", WHITE);
    printEntries(entryCount, memoryMap);
    parseMemoryMap(entryCount, memoryMap);
    
}

void printEntries(unsigned short entryCount, MemoryMapEntry* memoryMap)
{
    for (unsigned int i = 0; i < entryCount; i++) 
    {
        print("Memory Region: Base = ", WHITE);
        printNumber((unsigned int)memoryMap[i].base, WHITE);
        print(", Length = ", WHITE);
        printNumber((unsigned int)memoryMap[i].length, WHITE);
        print(", Type = ", WHITE);
        printNumber(memoryMap[i].type, WHITE);
        print("\n", WHITE);
    }
}

void printMemoryManagerInfo()
{
    unsigned int usableMB = (unsigned int)(memoryManager.totalUsable / (1024 * 1024));
    unsigned int reservedMB = (unsigned int)(memoryManager.totalReserved / (1024 * 1024));
     print("Total Usable Memory: ", WHITE);
    printNumber(usableMB, WHITE);
    print(" MB\n", WHITE);
    
    print("Total Reserved Memory: ", WHITE);
    printNumber(reservedMB, WHITE);
    print(" MB\n", WHITE);
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
            print("Warning: Memory region count exceeded!\n", RED);
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
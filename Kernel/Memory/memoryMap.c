#include "memoryMap.h"

static MemoryManager memoryManager;


void initializeMemoryManager()
{
    unsigned short entryCount = *((unsigned short*)MEMORY_MAP_ENTTRY_COUNT_ADDRESS);
    
    MemoryMapEntry* memoryMap = (MemoryMapEntry*)MEMORY_MAP_ADDRESS;    
    printNumber(entryCount, WHITE);
    if(entryCount) print("Memory map entries found.\n", WHITE);
    printEntries(entryCount, memoryMap);
    
}

void printEntries(unsigned short entryCount, MemoryMapEntry* memoryMap)
{
        for (unsigned int i = 0; i < entryCount; i++) 
    {
        //checking for each memory region type
        if (memoryMap[i].type == MEMORY_TYPE_AVAILABLE) 
        {
            memoryManager.totalUsable += memoryMap[i].length;
        } 
        else
        {
            memoryManager.totalReserved += memoryMap[i].length;
        }
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


void parseMemoryMap()
{
    memoryManager.regionCount = 0;
    memoryManager.totalReserved = 0;
    memoryManager.totalUsable = 0;

    
}
#include "memoryMap.h"

static MemoryManager memoryManager;
static MemoryMapEntry* memoryMap = (MemoryMapEntry*)MEMORY_MAP_ADDRESS;    

// Frame frames[TOTAL_FRAMES];
static Frame* frames;

void initializeMemoryManager()
{


    frames = (Frame*)0x00400000;
    //entry amounts getting from the specified memory address
    unsigned short entryCount = *((unsigned short*)MEMORY_MAP_ENTTRY_COUNT_ADDRESS);

    //printing memory map entry count
    // print("Memory Map Entry Count: ", WHITE);
    // printNumber(entryCount, WHITE);
    // print("\n", WHITE); 

    //checking if memory entries exist
    if(!entryCount) printLine("Warning: No memory map entries found..", YELLOW);

    //parsing memory map entries
    parseMemoryMap(entryCount, memoryMap);
    reserveKernelRegions();
    // printEntries();    

    //handling frames
    initializeFrames();

}


/*
-------------------
Handling Memory Map 
And Free and Reserved Regions
-------------------
*/

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
    
    print("Total Reserved Memory: <", WHITE);
    printNumber(reservedMB + 1, WHITE);
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
        region.isFree = (memoryMap[i].type == MEMORY_TYPE_AVAILABLE) ? TRUE : FALSE;


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
            // printLine("Reserved Memory Region Found", RED);
            // printNumber((unsigned int)memoryManager.totalReserved, WHITE);
            memoryManager.totalReserved += memoryMap[i].length;
        }
        //marking not free memory
        if(memoryMap[i].type != MEMORY_TYPE_AVAILABLE) 
        {
            region.isFree = FALSE;
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

    // first 1MB - BIOS/bootloader
    region.start  = 0x000000;
    region.end    = 0x100000;
    region.type   = MEMORY_TYPE_RESERVED;
    region.isFree = FALSE;
    memoryManager.regions[memoryManager.regionCount++] = region;

    // kernel + BSS
    region.start  = 0x100000;
    region.end    = 0x1A0000;
    region.type   = MEMORY_TYPE_RESERVED;
    region.isFree = FALSE;
    memoryManager.regions[memoryManager.regionCount++] = region;

    // heap
    region.start  = 0x1A0000;
    region.end    = 0x200000;
    region.type   = MEMORY_TYPE_RESERVED;
    region.isFree = FALSE;
    memoryManager.regions[memoryManager.regionCount++] = region;

    // stack
    region.start  = 0x200000;
    region.end    = 0x300000;
    region.type   = MEMORY_TYPE_RESERVED;
    region.isFree = FALSE;
    memoryManager.regions[memoryManager.regionCount++] = region;

    // paging structures
    region.start  = 0x300000;
    region.end    = 0x400000;
    region.type   = MEMORY_TYPE_RESERVED;
    region.isFree = FALSE;
    memoryManager.regions[memoryManager.regionCount++] = region;

    region.start  = 0xFEB00000;
    region.end    = 0xFEBFFFFF;
    region.type   = MEMORY_TYPE_RESERVED;
    region.isFree = FALSE;
    memoryManager.regions[memoryManager.regionCount++] = region;

    // frames array
    region.start  = 0x400000;
    region.end    = 0x440000;
    region.type   = MEMORY_TYPE_RESERVED;
    region.isFree = FALSE;
    memoryManager.regions[memoryManager.regionCount++] = region;
}


/*
-------------------
Handling Physical Memory Frames
-------------------
*/

void initializeFrames()
{
    //starting those frames as free
    for(unsigned int i = 0; i < TOTAL_FRAMES; i++)
    {
        frames[i].address = i * FRAME_SIZE;
        frames[i].isFree = TRUE;
    }
    settingReservedFrames();
}

void settingReservedFrames()
{
    for(unsigned int i = 0; i < memoryManager.regionCount; i++)
    {
        if(!memoryManager.regions[i].isFree)
        {
            unsigned long long startFrame = memoryManager.regions[i].start / FRAME_SIZE;
            unsigned long long endFrame = memoryManager.regions[i].end / FRAME_SIZE;
            for(unsigned long long j = startFrame; j < endFrame; j++)
            {

                frames[j].isFree = FALSE;
            }
        }
    }
}

int allocateFrame(unsigned long long address)
{
    unsigned long long frameIndex = address / FRAME_SIZE;
    // if (!frames[frameIndex].isFree) 
    // {
    //     printLine("Error: Frame already allocated!", RED);
    //     return ERROR;
    // }
    frames[frameIndex].isFree = FALSE;
    return frames[frameIndex].address;
}

int freeFrame(unsigned long long address)
{
    unsigned long long frameIndex = address / FRAME_SIZE;
    if (frames[frameIndex].isFree) 
    {
        printLine("Error: Frame is not allocated!", RED);
        return ERROR;
    }
    frames[frameIndex].isFree = TRUE;
    return frames[frameIndex].address;
}

unsigned long long allocateFreeFrame()
{
    for(unsigned int i = 0; i < TOTAL_FRAMES; i++)
    {
        unsigned long long frameAddress = frames[i].address;
        if(frames[i].isFree)
        {
            frames[i].isFree = FALSE;
            return frames[i].address;
        }
    }
    return ERROR; //no free frames available
}

int allocateFreeUserFrame()
{
    for(unsigned int i = FIRST_USER_FRAME; i < TOTAL_FRAMES; i++)
    {
        if(frames[i].isFree)
        {
            frames[i].isFree = FALSE;
            return frames[i].address;
        }
    }
    return ERROR; // no free frames
}
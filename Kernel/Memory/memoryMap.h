#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include "../Drivers/consoleDriver.h"

#define REGION_COUNT 128
#define MEMORY_MAP_ADDRESS 0x8004
#define MEMORY_MAP_ENTTRY_COUNT_ADDRESS 0x8000
#define MB 1024 * 1024
#define FRAME_SIZE 4096 //4KB
// #define TOTAL_FRAMES 1048576 //4GB / 4KB

//we map only 32 MB of memory for now
//because if we dont we will bloat the kernel size too much
//and bss section will be too big
//we can increase it later when we have a better memory management
//by using a bitmap or linked list
#define TOTAL_FRAMES (64 * MB) / FRAME_SIZE //mapping only 32 MB of memory for now


//the user addresses its a test it should work
#define FIRST_USER_FRAME ((0x400000) / FRAME_SIZE)

#define TRUE 1
#define FALSE 0
#define ERROR -1

enum RegionType 
{
    MEMORY_TYPE_AVAILABLE = 1,
    MEMORY_TYPE_RESERVED = 2,
    MEMORY_TYPE_ACPI_RECLAIMABLE = 3,
    MEMORY_TYPE_NVS = 4,
    MEMORY_TYPE_BAD_MEMORY = 5
};

typedef struct MemoryMapEntry
{
    unsigned long long base;
    unsigned long long length;
    unsigned int type;
    unsigned int acpi;
} __attribute__((packed)) MemoryMapEntry;


typedef struct MemoryRegion 
{
    unsigned long long start;
    unsigned long long end;
    unsigned int type;
    unsigned int isFree;
} MemoryRegion;



//each frame is 4KB
//so address is the starting address of the frame
typedef struct Frame
{
    unsigned long long address;
    unsigned int isFree;
} Frame;

typedef struct MemoryManager 
{
    MemoryRegion regions[REGION_COUNT];
    unsigned int regionCount;
    unsigned long long totalUsable;
    unsigned long long totalReserved;
} MemoryManager;


void initializeMemoryManager(void);


void parseMemoryMap(const unsigned short entryCount, MemoryMapEntry* memoryMap);
void printMemoryManagerInfo(void);
void printEntries(void);
void reserveKernelRegions(void);

void initializeFrames(void);
void settingReservedFrames(void);
int allocateFrame(unsigned long long address);
int freeFrame(unsigned long long address);
int allocateFreeFrame(void);
int allocateFreeUserFrame(void);
#endif
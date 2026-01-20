#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include "../Drivers/consoleDriver.h"

#define REGION_COUNT 128
#define MEMORY_MAP_ADDRESS 0x8004
#define MEMORY_MAP_ENTTRY_COUNT_ADDRESS 0x8000

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


typedef struct MemoryManager 
{
    MemoryRegion regions[REGION_COUNT];
    unsigned int regionCount;
    unsigned long long totalUsable;
    unsigned long long totalReserved;
} MemoryManager;


void parseMemoryMap(const unsigned short entryCount, MemoryMapEntry* memoryMap);
void initializeMemoryManager(void);
void printMemoryManagerInfo(void);
void printEntries(unsigned short entryCount, MemoryMapEntry* memoryMap);

#endif
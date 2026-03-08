#include "stats.h"

void showSystemStats() 
{
    printLineW("System Statistics:");
    printMemoryManagerInfo();
    printW("Free Space On Disk: ");
    printNumberW(CONVERT_TO_KB(getFreeBlocksCount()));
    printLineW(" KB");
}

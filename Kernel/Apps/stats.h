#ifndef STATS_H
#define STATS_H

#include "../Fs/superblock.h"   
#include "../Memory/memoryMap.h"

#define CONVERT_TO_KB(x) ((x * 512) / 1024)

void showSystemStats();

#endif

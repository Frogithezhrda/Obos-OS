#ifndef KERNEL_H
#define KERNEL_H
#include "../Drivers/consoleDriver.h"
#include "../Tables/IDT.h"
#include "../Tables/PIC.h"
#include "../Memory/heap.h"
#include "../SystemLib/obosMemory.h"
#include "../Fs/superblock.h"
#include "../Network/icmp.h"
void obos_main();

#endif  
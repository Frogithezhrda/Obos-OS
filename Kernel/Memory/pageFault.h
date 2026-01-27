#ifndef PAGE_FAULT_H
#define PAGE_FAULT_H


#include "../Drivers/consoleDriver.h"
#include "../SystemLib/errors.h"
#include "../Drivers/timerDriver.h"

enum PageFaultFlags {
    PF_PRESENT       = 0x1, // Page not present
    PF_WRITE         = 0x2, // Write operation
    PF_USER          = 0x4, // Processor was in user-mode
    PF_RESERVED      = 0x8, // Overwritten CPU-reserved bits of page entry
    PF_INSTRUCTION   = 0x10 // Caused by an instruction fetch
};

void pageFaultHandler(unsigned int errorCode);

#endif
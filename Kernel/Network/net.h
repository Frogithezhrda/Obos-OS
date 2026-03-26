#ifndef NET_H
#define NET_H

#include "pci.h"
#include "../Tables/PIC.h"
#include "../Memory/heap.h"
#include "rtl.h"


//those are default bus dev and func values for qemu

void initializeNet();

#endif
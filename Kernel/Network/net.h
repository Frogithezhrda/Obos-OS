#ifndef NET_H
#define NET_H

#include "pci.h"
#include "../Tables/PIC.h"
#include "../Memory/heap.h"
#include "rtl.h"

#define RTL8139_BUS 0
#define RTL8139_DEV 3
#define RTL8139_FUNC 0

#define RX_BUFFER 8206

void initializeNet();

#endif
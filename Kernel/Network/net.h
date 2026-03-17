#ifndef NET_H
#define NET_H

#include "netdevice.h"
#include "icmp.h"
#include "pci.h"
#include "e1000.h"

#define E1000_BUS 0
#define E1000_DEV 3
#define E1000_FUNC 0

void initializeNet();

#endif
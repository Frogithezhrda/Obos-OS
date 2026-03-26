#ifndef RTL_H
#define RTL_H

#include "../Tables/IDT.h"
#include "netdevice.h"
#include "../SystemLib/obosMemory.h"
#include "ethernet.h"

#define RTL8139_BUS 0
#define RTL8139_DEV 3
#define RTL8139_FUNC 0

#define RX_BUFFER (8192 + 16 + 1500)

void rtlInitialize();
void RTL8139ISRHandler();
void rtlReceivePacket();
int rtlNetSend(NetDevice* dev, const void* data, unsigned int len);

#endif // RTL_H
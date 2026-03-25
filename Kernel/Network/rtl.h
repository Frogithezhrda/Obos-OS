#ifndef RTL_H
#define RTL_H

#include "../Tables/IDT.h"

void RTL8139ISRHandler();
void rtlSendPacket(unsigned int length, void* data);
void rtlReceivePacket();

extern unsigned short ioaddrRtl; 

#endif // RTL_H
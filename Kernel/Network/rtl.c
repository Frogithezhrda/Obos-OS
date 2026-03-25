#include "rtl.h"

unsigned short ioaddrRtl;

void RTL8139ISRHandler()
{
    unsigned short status = inw(ioaddrRtl + 0x3E);

    outw(ioaddrRtl + 0x3E, status);

    if (status & 0x01) 
    {
        printW("Packet received!");
    }

    if (status & 0x04) 
    {
        printW("Packet sent!");
    }

    endOfInterrupt(11);
}

void rtlSendPacket(unsigned int length, void* data)
{
    outl(ioaddrRtl + 0x20, (unsigned int)data);
    outl(ioaddrRtl + 0x10, length);

    while (!(inl(ioaddrRtl + 0x10) & 32768));

    printW("Packet sent (polling)!");
}

void rtlReceivePacket()
{

}

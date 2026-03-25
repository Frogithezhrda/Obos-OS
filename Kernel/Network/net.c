#include "net.h"

char* rxBuffer = {0};

char test_packet[64] = {
    // Destination MAC (broadcast)
    0xff,0xff,0xff,0xff,0xff,0xff,

    // Source MAC (fake for now)
    0x12,0x34,0x56,0x78,0x9A,0xBC,

    // EtherType (0x0800 = IPv4)
    0x08,0x00,

    // Payload (fill with something)
};

void sendTestPacket()
{
    // Fill payload so packet is at least 60 bytes
    for (int i = 14; i < 60; i++) {
        test_packet[i] = i;
    }

    rtlSendPacket(60, test_packet);

    printW("Test packet sent!");
}

void initializeNet()
{

    unsigned short ioaddr = pciGetBar0(RTL8139_BUS, RTL8139_DEV, RTL8139_FUNC);
    if (ioaddr == 0) 
    {
        printW("RTL8139 BAR0 not found!");
        return;
    }
    outb(ioaddr + 0x52, 0x0);
    outb(ioaddr + 0x37, 0x10);
    while( (inb(ioaddr + 0x37) & 0x10) != 0);
    rxBuffer = (char*)kmalloc(RX_BUFFER);
    outl(ioaddr + 0x30, (unsigned int)rxBuffer);
    outw(ioaddr + 0x3C, 0x0005);
    outl(ioaddr + 0x44, 0x0F | 128);
    outb(ioaddr + 0x37, 0x0C);
    printW("RTL8139 initialized!");
    ioaddrRtl = ioaddr;

    // for (volatile int i = 0; i < 10000000; i++);
    // ethernetInit(&eth0);
    // arpInit(&eth0);
    // ipInit(&eth0);
    // icmpInit(&eth0);
}
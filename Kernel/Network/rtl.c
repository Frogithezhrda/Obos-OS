#include "rtl.h"

static unsigned int rxOffset = 0;
static NetDevice rtlDev;
char* rxBuffer = {0};
static unsigned short ioaddr = 0;
static int txIndex = 0;

static void printMAC(unsigned char* mac)
{
    char hexChars[] = "0123456789ABCDEF";

    for (int i = 0; i < 6; i++)
    {
        printCharW(hexChars[(mac[i] >> 4) & 0xF]);
        printCharW(hexChars[mac[i] & 0xF]);

        if (i < 5) printCharW(':');
    }
}

void RTL8139ISRHandler()
{
    unsigned short status = inw(ioaddr + 0x3E);

    outw(ioaddr + 0x3E, status);

    if (status & 0x01) 
    {
        printLineW("Packet received!!!");
        rtlReceivePacket();
    }

    if (status & 0x04) 
    {
        printLineW("Packet sent!!!");
    }

    endOfInterrupt(11);
}

static void rtlSendPacket(unsigned int length, void* data)
{
    int entry = txIndex;

    while (!(inl(ioaddr + 0x10 + entry * 4) & (1 << 13)));
    outl(ioaddr + 0x20 + entry * 4, (unsigned int)data);
    outl(ioaddr + 0x10 + entry * 4, length);
    while (!(inl(ioaddr + 0x10) & 32768));
    txIndex = (txIndex + 1) % 4;
    printLineW("Packet sent (polling)!");
}

void rtlReceivePacket()
{
    while ((inb(ioaddr + 0x37) & 0x01) == 0)
    {
        char* ptr = (char*)(rxBuffer + rxOffset);

        unsigned short status = *(unsigned short*)(ptr);
        unsigned short length = *(unsigned short*)(ptr + 2);
        if (!(status & 0x01))
        {
            printLineW("Bad packet!");
            break;
        }

        char* data = ptr + 4;

        printLineW("Packet received!");
        netDeviceReceive(&rtlDev, data, length);

        rxOffset = (rxOffset + length + 4 + 3) & ~3;

        rxOffset %= 8192;

        outw(ioaddr + 0x38, (rxOffset - 16) & 0xFFFF);

    }
}

int rtlNetSend(NetDevice* dev, const void* data, unsigned int len)
{
    rtlSendPacket(len, (void*)data);
    return SUCCESS;
}

void rtlInitialize()
{
    ioaddr = pciGetBar0(RTL8139_BUS, RTL8139_DEV, RTL8139_FUNC);
    if (ioaddr == 0) 
    {
        printW("RTL8139 BAR0 not found!");
        return;
    }
    outb(ioaddr + 0x52, 0x0);
    outb(ioaddr + 0x37, 0x10);
    while ((inb(ioaddr + 0x37) & 0x10));
    rxBuffer = (char*)kmalloc(RX_BUFFER);
    outl(ioaddr + 0x30, (unsigned int)rxBuffer);
    outw(ioaddr + 0x38, 0xFFF0);
    outw(ioaddr + 0x3C, 0x0005);
    outl(ioaddr + 0x44, 0x0F | 128);
    outb(ioaddr + 0x37, 0x0C);
    printLineW("RTL8139 initialized!");


    rtlDev.send = rtlNetSend;
    rtlDev.rx = ethernetReceive;

    memcpy(rtlDev.name, "eth0", 5);
    for (int i = 0; i < 6; i++)
    {
        rtlDev.mac[i] = inb(ioaddr + i);
    }
    printMAC(rtlDev.mac);
    
    netDeviceRegister(&rtlDev);
}
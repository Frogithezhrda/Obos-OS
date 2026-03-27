#include "pci.h"

unsigned int pciConfigRead(unsigned char bus, unsigned char device, unsigned char func, unsigned char offset)
{
    unsigned int address = (1 << 31)
                         | ((unsigned int)bus << 16)
                         | ((unsigned int)device << 11)
                         | ((unsigned int)func <<  8)
                         | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pciConfigWrite(unsigned char bus, unsigned char device, unsigned char func, unsigned char offset, unsigned int value)
{
    unsigned int address = (1 << 31)
                     | (bus << 16)
                     | (device << 11)
                     | (func << 8)
                     | (offset & 0xFC);

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}
unsigned short pciGetBar0(unsigned char bus, unsigned char device, unsigned char func)
{
    unsigned int bar0 = pciConfigRead(bus, device, func, 0x10);
    unsigned int cmd = pciConfigRead(bus, device, func, 0x04);
    cmd |= 4;
    pciConfigWrite(bus, device, func, 0x04, cmd);
    if (cmd & 4) 
    {
        printLine("Bus Mastering ENABLED", YELLOW);
    } 
    else 
    {
        printLine("Bus Mastering DISABLED\n", YELLOW);
    }
    // bit 0 = 0 means memory mapped, mask the lower 4 flag bits
    if (bar0 & 1)
    {
        return (unsigned short)(bar0 & ~0x3);
    }
    return 0;
}
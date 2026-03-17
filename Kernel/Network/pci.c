#include "pci.h"

unsigned int pciConfigRead(unsigned char bus, unsigned char device, unsigned char func, unsigned char offset)
{
    unsigned int address = (-2147483648u)
                         | ((unsigned int)bus << 16)
                         | ((unsigned int)device << 11)
                         | ((unsigned int)func <<  8)
                         | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void* pciGetBar0(unsigned char bus, unsigned char device, unsigned char func)
{
    unsigned int bar0 = pciConfigRead(bus, device, func, 0x10);
    // bit 0 = 0 means memory mapped, mask the lower 4 flag bits
    return (void*)(bar0 & 0xFFFFFFF0);
}
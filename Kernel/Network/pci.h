#ifndef PCI_H
#define PCI_H

#include "../Drivers/consoleDriver.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

unsigned int pciConfigRead(unsigned char bus, unsigned char device, unsigned char func, unsigned char offset);
void pciConfigWrite(unsigned char bus, unsigned char device, unsigned char func, unsigned char offset, unsigned int value);

unsigned short pciGetBar0(unsigned char bus, unsigned char device, unsigned char func);

#endif
 
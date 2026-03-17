#ifndef PCI_H
#define PCI_H

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

unsigned int pciConfigRead(unsigned char bus, unsigned char device, unsigned char func, unsigned char offset);
 
void* pciGetBar0(unsigned char bus, unsigned char device, unsigned char func);

#endif
 
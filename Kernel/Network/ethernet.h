#ifndef ETHERNET_H
#define ETHERNET_H

#include "netdevice.h"
#include "byteorder.h"

#define ETHERNET_HEADER_SIZE 14

#define ETHERTYPE_IP    0x0800
#define ETHERTYPE_ARP   0x0806

typedef struct
{
    unsigned char dest[6];
    unsigned char src[6];
    unsigned short type;
} __attribute__((packed)) EthernetHeader;

void ethernetInit(NetDevice* dev);
void ethernetSend(unsigned char* dstMac, unsigned short ethertype, void* data, unsigned int length);
void ethernetReceive(void* buffer, unsigned int length);

#endif // ETHERNET_H
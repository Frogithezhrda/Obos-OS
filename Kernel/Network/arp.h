#ifndef ARP_H
#define ARP_H

#include "netdevice.h"
#include "byteorder.h"

#define ARP_HTYPE_ETHERNET 1
#define ARP_PTYPE_IP       0x0800
#define ARP_HLEN           6
#define ARP_PLEN           4
#define ARP_OP_REQUEST     1
#define ARP_OP_REPLY       2

#define ARP_CACHE_SIZE 16
#define MY_IP 0x0A00020F //default qemu ip 10.0.2.15
#define QEMU_GATEWAY 0x0A000202 //default qemu gateway 10.0.2.2
#define ETHERTYPE_IP    0x0800
#define ETHERTYPE_ARP   0x0806

typedef struct ArpPacket
{
    unsigned short htype;
    unsigned short ptype;
    unsigned char  hlen;
    unsigned char  plen;
    unsigned short op;
    unsigned char  senderMac[6];
    unsigned int   senderIp;
    unsigned char  targetMac[6];
    unsigned int   targetIp;
} __attribute__((packed)) ArpPacket;

typedef struct ArpEntry
{
    unsigned int  ip;
    unsigned char mac[6];
    int           valid;
} ArpEntry;

void arpInit(NetDevice* dev);
 
void arpRequest(unsigned int ip);
 
void arpReceive(void* data, unsigned int length);
 
unsigned char* arpLookup(unsigned int ip);

extern unsigned int myIP;

#endif
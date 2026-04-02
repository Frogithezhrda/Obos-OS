#ifndef UDP_H
#define UDP_H

#include "ip.h"

#define UDP_PROTOCOL 17

//protocols
#define DNS_PORT 53
#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

#define UDP_HEADER_SIZE 8

typedef struct UdpHeader
{
    unsigned short srcPort;
    unsigned short destPort;
    unsigned short length;
    unsigned short checksum;
} __attribute__((packed)) UdpHeader;


void udpInit(NetDevice* dev);
void udpSend(unsigned int srcIp, unsigned int dstIp, unsigned short srcPort, unsigned short dstPort, void* data, unsigned int length);
void udpReceive(NetDevice* dev, const void* buffer, unsigned int length);

unsigned short udpChecksum(unsigned int srcIp, unsigned int dstIp, void* data, unsigned int length);

extern NetDevice* udpDev;

#endif
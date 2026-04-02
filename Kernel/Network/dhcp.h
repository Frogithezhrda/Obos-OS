#ifndef DHCP_H
#define DHCP_H

#include "udp.h"

#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

#define DHCP_BROADCAST_IP 0xFFFFFFFF 
#define DHCP_SRC_IP 0x00000000 //clients should use
typedef struct DhcpPacket
{
    unsigned char op;
    unsigned char htype;
    unsigned char hlen;
    unsigned char hops;
    unsigned int xid;
    unsigned short secs;
    unsigned short flags;
    unsigned int ciaddr;
    unsigned int yiaddr;
    unsigned int siaddr;
    unsigned int giaddr;
    unsigned char chaddr[16];
    unsigned char sname[64];
    unsigned char file[128];
} __attribute__((packed)) DhcpPacket;

void discoverDhcp();
void requestDhcp();
void receiveDhcp(void* data, unsigned int length);

#endif
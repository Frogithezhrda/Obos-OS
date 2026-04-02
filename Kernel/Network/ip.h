#ifndef IP_H
#define IP_H

#include "arp.h"

#define IP_PROTO_ICMP 1
#define IP_PROTO_UDP 17
#define IP_PROTO_DHCP 67

#define IP_TTL 64
#define SUBNET_MASK 0xFFFFFF00
#define SUBNET 0x0A000200

typedef struct IpHeader
{
    unsigned char versionIhl;
    unsigned char tos;
    unsigned short totalLength;
    unsigned short id; 
    unsigned short flagsFragment;
    unsigned char ttl;    
    unsigned char protocol;      
    unsigned short checksum;      
    unsigned int srcIp;
    unsigned int dstIp;
} __attribute__((packed)) IpHeader;
 
#define IP_HEADER_SIZE sizeof(IpHeader)
 
void ipInit(NetDevice* dev);
 
void ipSend(unsigned int dstIp, unsigned char protocol, void* data, unsigned int length);
 
void ipReceive(void* data, unsigned int length);

unsigned short checksum(void* data, unsigned int length);

unsigned int splitIP(unsigned char* ip);

unsigned char* ipRoute(unsigned int ip);

extern unsigned int subnetMask;
extern unsigned int subnet;

#endif
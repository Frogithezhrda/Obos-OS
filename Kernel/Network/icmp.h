#ifndef ICMP_H
#define ICMP_H
 
#include "ip.h"
#include "../Drivers/consoleDriver.h"

#define ICMP_TYPE_ECHO_REQUEST 8
#define ICMP_TYPE_ECHO_REPLY   0
#define ICMP_CODE_ZERO         0
 
typedef struct ICMPHeader
{
    unsigned char  type;
    unsigned char  code;
    unsigned short checksum;
    unsigned short id;
    unsigned short sequence;
} __attribute__((packed)) ICMPHeader;
 
#define ICMP_HEADER_SIZE sizeof(ICMPHeader)
 
void icmpInit(NetDevice* dev);
 
void icmpReceive(void* data, unsigned int length, unsigned int srcIp);
 
void icmpSendEchoRequest(unsigned int dstIp);
 
#endif
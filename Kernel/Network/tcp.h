#ifndef TCP_H
#define TCP_H

#include "ip.h"

#define TCP_PROTOCOL 6



/*
Not Doing this currently TCP is really complex
and i want to work on something more basic like graphic switching
this is delayed for later.
*/
typedef struct TcpHeader
{
    unsigned short srcPort;
    unsigned short destPort;
    unsigned int seqNum;
    unsigned int ackNum;
    unsigned char dataOffset; //4 bits
    unsigned char flags; //6 bits
    unsigned short windowSize;
    unsigned short checksum;
    unsigned short urgentPointer;
} __attribute__((packed)) TcpHeader;


void tcpInit(NetDevice* dev);
void tcpSend(unsigned int srcIp, unsigned int dstIp, unsigned short srcPort, unsigned short dstPort, void* data, unsigned int length);
void tcpReceive(NetDevice* dev, const void* buffer, unsigned int length);


#endif
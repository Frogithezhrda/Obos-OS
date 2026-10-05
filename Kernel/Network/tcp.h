#ifndef TCP_H
#define TCP_H

#include "ip.h"

#define TCP_PROTOCOL 6

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10


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

typedef enum
{
    TCP_CLOSED, TCP_SYN_SENT, TCP_ESTABLISHED,
    TCP_FIN_WAIT_1, TCP_FIN_WAIT_2, TCP_CLOSE_WAIT, TCP_LAST_ACK
} TcpState;

typedef void (*TcpDataCb)(const unsigned char* data, unsigned int len);
typedef void (*TcpCloseCb)(void);

void tcpInit(NetDevice* dev);
int tcpConnect(unsigned int dstIp, unsigned short dstPort, TcpDataCb onData, TcpCloseCb onClose);
int tcpSend(void* data, unsigned int length);
void tcpReceive(NetDevice* dev, const void* buffer, unsigned int length);
void tcpClose();
void tcpTick();
TcpState tcpGetState();

#endif
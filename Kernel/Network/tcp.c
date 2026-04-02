#include "tcp.h"

static NetDevice* tcpDev;


void tcpInit(NetDevice* dev)
{
    tcpDev = dev;
}
void tcpSend(unsigned int srcIp, unsigned int dstIp, unsigned short srcPort, unsigned short dstPort, void* data, unsigned int length)
{
    
}
void tcpReceive(NetDevice* dev, const void* buffer, unsigned int length)
{
    // Implementation for receiving TCP packets
}

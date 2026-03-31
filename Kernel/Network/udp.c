#include "udp.h"

static NetDevice* udpDev = 0;

void udpInit(NetDevice* dev)
{
    udpDev = dev;
}

void udpSend(unsigned int srcIp, unsigned int dstIp, unsigned short srcPort, unsigned short dstPort, void* data, unsigned int length)
{
    if (!udpDev)
    {
        return;
    }
    unsigned char frame[ETH_FRAME_MAX];
    UdpHeader* hdr = (UdpHeader*)frame;

    hdr->srcPort = htons(srcPort);
    hdr->destPort = htons(dstPort);
    hdr->length = htons(UDP_HEADER_SIZE + length);
    hdr->checksum = udpChecksum(srcIp, dstIp, data, length);

    unsigned char* payload = frame + UDP_HEADER_SIZE;
    unsigned char* src = (unsigned char*)data;

    for (unsigned int i = 0; i < length; i++)
    {
        payload[i] = src[i];
    }

    ipSend(dstIp, UDP_PROTOCOL, frame, UDP_HEADER_SIZE + length);
}

void udpReceive(NetDevice* dev, const void* buffer, unsigned int length)
{

}

unsigned short udpChecksum(unsigned int srcIp, unsigned int dstIp, void* data, unsigned int length)
{
    unsigned int sum = 0;
    unsigned short* ptr = (unsigned short*)data;
    sum += (srcIp >> 16) & 0xFFFF;
    sum += (srcIp) & 0xFFFF;

    sum += (dstIp >> 16) & 0xFFFF;
    sum += (dstIp) & 0xFFFF;
    sum += UDP_PROTOCOL;
    sum += length;

    while (length > 1) 
    {
        sum += *ptr++;
        length -= 2;
    }
    //if one left then add
    if (length > 0) 
    {
        sum += *((unsigned char*)ptr);
    }

    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (unsigned short)~sum;
}

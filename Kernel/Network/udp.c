#include "udp.h"
#include "dns.h"

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
    hdr->checksum = 0;
    unsigned char* payload = frame + UDP_HEADER_SIZE;
    unsigned char* src = (unsigned char*)data;

    for (unsigned int i = 0; i < length; i++)
    {
        payload[i] = src[i];
    }
    hdr->checksum = udpChecksum(srcIp, dstIp, frame, UDP_HEADER_SIZE + length);

    ipSend(dstIp, UDP_PROTOCOL, frame, UDP_HEADER_SIZE + length);
}

void udpReceive(NetDevice* dev, const void* buffer, unsigned int length)
{
    /*
    Receving UDP packets is a bit more complicated than sending because we need to parse the header and then pass the payload to the application that is listening on the destination port
    but! we dont have any like application listening so. we will just let the kernel handle the packet

    
    */
    if (length < UDP_HEADER_SIZE)
    {
        return;
    }
    UdpHeader* hdr = (UdpHeader*)buffer;
    unsigned short srcPort = ntohs(hdr->srcPort);
    unsigned short destPort = ntohs(hdr->destPort);
    unsigned short dataLength = ntohs(hdr->length) - UDP_HEADER_SIZE;
    unsigned char* payload = (unsigned char*)buffer + UDP_HEADER_SIZE;

    if(ntohs(hdr->srcPort) == DNS_PORT)
    {
        dnsReceive(payload, dataLength);
    }

}

unsigned short udpChecksum(unsigned int srcIp, unsigned int dstIp, void* data, unsigned int length)
{
    unsigned int sum = 0;
    srcIp = htonl(srcIp);
    dstIp = htonl(dstIp);
    // pseudo header
    sum += (srcIp >> 16) & 0xFFFF;
    sum += (srcIp)       & 0xFFFF;
    sum += (dstIp >> 16) & 0xFFFF;
    sum += (dstIp)       & 0xFFFF;
    sum += htons(UDP_PROTOCOL);
    sum += htons(length); // full UDP length

    // UDP header + payload
    unsigned short* ptr = (unsigned short*)data;
    unsigned int remaining = length;
    while (remaining > 1)
    {
        sum += *ptr++;
        remaining -= 2;
    }
    if (remaining == 1)
        sum += *(unsigned char*)ptr;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (unsigned short)~sum;
}

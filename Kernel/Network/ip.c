#include "ip.h"
#include "icmp.h"

static NetDevice* ipDev = 0;
 
void ipInit(NetDevice* dev)
{
    ipDev = dev;
}
 
unsigned short checksum(void* data, unsigned int length)
{
    unsigned short* ptr = (unsigned short*)data;
    unsigned int sum = 0;
    while (length > 1)
    {
        sum += *ptr;
        ptr++;
        length -= 2;
    }
    if (length == 1)
    {
        sum += *(unsigned char*)ptr;
    }
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (unsigned short)(~sum);
}
 
void ipSend(unsigned int dstIp, unsigned char protocol, void* data, unsigned int length)
{
    if (!ipDev)
    {
        return;
    }
    unsigned char* dstMac = arpLookup(dstIp);
    if (!dstMac)
    {
        arpRequest(dstIp);
        return;
    }
    unsigned char packet[ETH_FRAME_MAX];
    IpHeader* hdr = (IpHeader*)packet;
    hdr->versionIhl    = (4 << 4) | 5;
    hdr->tos           = 0;
    hdr->totalLength   = htons((unsigned short)(IP_HEADER_SIZE + length));
    hdr->id            = 0;
    hdr->flagsFragment = 0;
    hdr->ttl           = IP_TTL;
    hdr->protocol      = protocol;
    hdr->checksum      = 0;
    hdr->srcIp         = htonl(myIP);
    hdr->dstIp         = htonl(dstIp);
    hdr->checksum      = checksum(hdr, IP_HEADER_SIZE);
    unsigned char* payload = packet + IP_HEADER_SIZE;
    unsigned char* src = (unsigned char*)data;
    for (unsigned int i = 0; i < length; i++)
    {
        payload[i] = src[i];
    }
    ethernetSend(dstMac, ETHERTYPE_IP, packet, IP_HEADER_SIZE + length);
}
 
void ipReceive(void* data, unsigned int length)
{
    if (length < IP_HEADER_SIZE)
    {
        return;
    }
    IpHeader* hdr = (IpHeader*)data;
    void* payload = (unsigned char*)data + IP_HEADER_SIZE;
    unsigned int paylen = ntohs(hdr->totalLength) - IP_HEADER_SIZE;
    if (hdr->protocol == IP_PROTO_ICMP)
    {
        icmpReceive(payload, paylen, myIP);
    }
}

#include "ethernet.h"

static NetDevice* ethDev = NULL;

void ethernetInit(NetDevice* dev)
{
    ethDev = dev;
    netDeviceInit(ethDev);
}

void ethernetSend(unsigned char* dstMac, unsigned short ethertype, void* data, unsigned int length)
{
    if (ethDev)
    {
        PacketBuffer* pkt = packetBufferAlloc();
        if (!pkt) return;

        EthernetHeader* hdr = (EthernetHeader*)pkt->data;
        memcpy(hdr->dest, dstMac,      6);
        memcpy(hdr->src,  ethDev->mac, 6);
        hdr->type = htons(ethertype);

        memcpy(pkt->data + ETHERNET_HEADER_SIZE, data, length);
        pkt->length = ETHERNET_HEADER_SIZE + length;

        netDeviceTransmit(ethDev, pkt);
    }
}

void ethernetReceive(void* buffer, unsigned int length)
{
    if (length < ETHERNET_HEADER_SIZE) return;

    EthernetHeader* hdr = (EthernetHeader*)buffer;
    unsigned short type = ntohs(hdr->type);

    void* payload = (unsigned char*)buffer + ETHERNET_HEADER_SIZE;
    unsigned int paylen = length - ETHERNET_HEADER_SIZE;

    switch (type)
    {
        case ETHERTYPE_ARP:
            // arpReceive(payload, paylen);
            break;
        case ETHERTYPE_IP:
            // ipReceive(payload, paylen);
            break;
        default:
            break;
    }
}

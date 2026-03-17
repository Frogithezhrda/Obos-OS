#include "ethernet.h"

static NetDevice* ethDev = NULL;

void ethernetInit(NetDevice* dev)
{
    ethDev = dev;
    ethDev->rx = ethernetReceive;
}

void ethernetSend(unsigned char* dstMac, unsigned short ethertype, void* data, unsigned int length)
{
    if (!ethDev)
    {
        return;
    }
    printLineW("ether sended");
    unsigned char frame[ETH_FRAME_MAX];
    EthernetHeader* hdr = (EthernetHeader*)frame;
    // destination mac
    hdr->dest[0] = dstMac[0];
    hdr->dest[1] = dstMac[1];
    hdr->dest[2] = dstMac[2];
    hdr->dest[3] = dstMac[3];
    hdr->dest[4] = dstMac[4];
    hdr->dest[5] = dstMac[5];
    // source mac (our own mac from the device)
    hdr->src[0] = ethDev->mac[0];
    hdr->src[1] = ethDev->mac[1];
    hdr->src[2] = ethDev->mac[2];
    hdr->src[3] = ethDev->mac[3];
    hdr->src[4] = ethDev->mac[4];
    hdr->src[5] = ethDev->mac[5];
    // ethertype goes on the wire in big endian
    hdr->type = htons(ethertype);
    // copy the payload right after the header
    unsigned char* payload = frame + ETHERNET_HEADER_SIZE;
    unsigned char* src = (unsigned char*)data;
    for (unsigned int i = 0; i < length; i++)
    {
        payload[i] = src[i];
    }
    netDeviceSend(ethDev, frame, ETHERNET_HEADER_SIZE + length);
}

void ethernetReceive(NetDevice* dev, const void* buffer, unsigned int length)
{
    printLineW("ravos");
    if (length < ETHERNET_HEADER_SIZE) return;

    EthernetHeader* hdr = (EthernetHeader*)buffer;
    unsigned short type = ntohs(hdr->type);

    void* payload = (unsigned char*)buffer + ETHERNET_HEADER_SIZE;
    unsigned int paylen = length - ETHERNET_HEADER_SIZE;
    switch (type)
    {
        case ETHERTYPE_ARP:
            arpReceive(payload, paylen);
            break;
        case ETHERTYPE_IP:
            ipReceive(payload, paylen);
            break;
        default:
            break;
    }
}

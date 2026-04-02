#include "arp.h"
#include "ethernet.h"
#include "../Drivers/consoleDriver.h"

static NetDevice* arpDev = 0;
static ArpEntry arpCache[ARP_CACHE_SIZE];
unsigned int myIP = 0x00000000;
unsigned int gateway = 0xFFFFFFFF;

void printIP(unsigned int ip)
{
    printNumberW((ip >> 24) & 0xFF);
    printW(".");
    printNumberW((ip >> 16) & 0xFF);
    printW(".");
    printNumberW((ip >> 8) & 0xFF);
    printW(".");
    printNumberW(ip & 0xFF);
}

void arpInit(NetDevice* dev)
{
    arpDev = dev;
}

void arpRequest(unsigned int ip)
{
    if (!arpDev)
    {
        return;
    }
    ArpPacket pkt;
    pkt.htype = htons(ARP_HTYPE_ETHERNET);
    pkt.ptype = htons(ARP_PTYPE_IP);
    pkt.hlen = ARP_HLEN;
    pkt.plen = ARP_PLEN;
    pkt.op = htons(ARP_OP_REQUEST);
    // sender is me
    pkt.senderMac[0] = arpDev->mac[0];
    pkt.senderMac[1] = arpDev->mac[1];
    pkt.senderMac[2] = arpDev->mac[2];
    pkt.senderMac[3] = arpDev->mac[3];
    pkt.senderMac[4] = arpDev->mac[4];
    pkt.senderMac[5] = arpDev->mac[5];
    pkt.senderIp = htonl(myIP);

    pkt.targetMac[0] = 0;
    pkt.targetMac[1] = 0;
    pkt.targetMac[2] = 0;
    pkt.targetMac[3] = 0;
    pkt.targetMac[4] = 0;
    pkt.targetMac[5] = 0;
    pkt.targetIp = htonl(ip);
    unsigned char broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ethernetSend(broadcast, ETHERTYPE_ARP, &pkt, sizeof(ArpPacket));
}

void arpCacheInsert(unsigned int ip, unsigned char* mac)
{
    for (int i = 0; i < ARP_CACHE_SIZE; i++)
    {
        if (arpCache[i].valid && arpCache[i].ip == ip)
        {
            arpCache[i].mac[0] = mac[0];
            arpCache[i].mac[1] = mac[1];
            arpCache[i].mac[2] = mac[2];
            arpCache[i].mac[3] = mac[3];
            arpCache[i].mac[4] = mac[4];
            arpCache[i].mac[5] = mac[5];
            return;
        }
    }
    for (int i = 0; i < ARP_CACHE_SIZE; i++)
    {
        if (!arpCache[i].valid)
        {
            arpCache[i].ip = ip;
            arpCache[i].mac[0] = mac[0];
            arpCache[i].mac[1] = mac[1];
            arpCache[i].mac[2] = mac[2];
            arpCache[i].mac[3] = mac[3];
            arpCache[i].mac[4] = mac[4];
            arpCache[i].mac[5] = mac[5];
            arpCache[i].valid = 1;
            return;
        }
    }
}

void arpReceive(void* data, unsigned int length)
{
    if (length < sizeof(ArpPacket))
    {
        return;
    }
    ArpPacket* pkt = (ArpPacket*)data;
    unsigned short op = ntohs(pkt->op);
    unsigned int senderIp = ntohl(pkt->senderIp);

    arpCacheInsert(senderIp, pkt->senderMac);

    if (op == ARP_OP_REQUEST && ntohl(pkt->targetIp) == myIP)
    {
        ArpPacket reply;
        reply.htype = htons(ARP_HTYPE_ETHERNET);
        reply.ptype = htons(ARP_PTYPE_IP);
        reply.hlen = ARP_HLEN;
        reply.plen = ARP_PLEN;
        reply.op = htons(ARP_OP_REPLY);
        // we are the sender
        reply.senderMac[0] = arpDev->mac[0];
        reply.senderMac[1] = arpDev->mac[1];
        reply.senderMac[2] = arpDev->mac[2];
        reply.senderMac[3] = arpDev->mac[3];
        reply.senderMac[4] = arpDev->mac[4];
        reply.senderMac[5] = arpDev->mac[5];
        reply.senderIp  = htonl(myIP);
        // target is whoever asked
        reply.targetMac[0] = pkt->senderMac[0];
        reply.targetMac[1] = pkt->senderMac[1];
        reply.targetMac[2] = pkt->senderMac[2];
        reply.targetMac[3] = pkt->senderMac[3];
        reply.targetMac[4] = pkt->senderMac[4];
        reply.targetMac[5] = pkt->senderMac[5];
        reply.targetIp  = pkt->senderIp;
        ethernetSend(pkt->senderMac, ETHERTYPE_ARP, &reply, sizeof(ArpPacket));
    }
}
 
unsigned char* arpLookup(unsigned int ip)
{
    for (int i = 0; i < ARP_CACHE_SIZE; i++)
    {
        if (arpCache[i].valid && arpCache[i].ip == ip)
        {
            return arpCache[i].mac;
        }
    }
    return 0;
}

void arpPrintCache()
{
    printLineW("ARP Cache:");
    for (int i = 0; i < ARP_CACHE_SIZE; i++)
    {
        if (arpCache[i].valid)
        {
            printW("  IP: ");
            printIP(arpCache[i].ip);
            printW(" MAC: ");
            printMAC(arpCache[i].mac);
        }
    }
}
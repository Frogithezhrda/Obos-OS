#include "icmp.h"

static NetDevice* icmpDev = 0;
static unsigned short icmpSeq = 0;

void icmpInit(NetDevice* dev)
{
    icmpDev = dev;
}

void icmpSendEchoRequest(unsigned int dstIp)
{
    unsigned char packet[ICMP_HEADER_SIZE];
    ICMPHeader* hdr = (ICMPHeader*)packet;
    hdr->type = ICMP_TYPE_ECHO_REQUEST;
    hdr->code = ICMP_CODE_ZERO;
    hdr->checksum = 0;
    hdr->id = htons(1);
    hdr->sequence = htons(icmpSeq++);
    hdr->checksum = checksum(hdr, ICMP_HEADER_SIZE);
    ipSend(dstIp, IP_PROTO_ICMP, packet, ICMP_HEADER_SIZE);
}

void icmpReceive(void* data, unsigned int length, unsigned int srcIp)
{
    if (length < ICMP_HEADER_SIZE)
    {
        return;
    }
    ICMPHeader* hdr = (ICMPHeader*)data;
    if (hdr->type == ICMP_TYPE_ECHO_REQUEST)
    {
        unsigned char packet[ETH_FRAME_MAX];
        ICMPHeader* reply = (ICMPHeader*)packet;
        reply->type     = ICMP_TYPE_ECHO_REPLY;
        reply->code     = ICMP_CODE_ZERO;
        reply->checksum = 0;
        reply->id       = hdr->id;
        reply->sequence = hdr->sequence;

        unsigned int paylen = length - ICMP_HEADER_SIZE;
        unsigned char* replyPayload = packet + ICMP_HEADER_SIZE;
        unsigned char* srcPayload = (unsigned char*)data + ICMP_HEADER_SIZE;
        for (unsigned int i = 0; i < paylen; i++)
        {
            replyPayload[i] = srcPayload[i];
        }
        reply->checksum = checksum(packet, ICMP_HEADER_SIZE + paylen);
        ipSend(srcIp, IP_PROTO_ICMP, packet, ICMP_HEADER_SIZE + paylen);
    }
    if (hdr->type == ICMP_TYPE_ECHO_REPLY)
    {
        printLine("pong!", LIGHT_GREEN);
    }
}
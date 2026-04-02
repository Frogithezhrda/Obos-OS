#include "dhcp.h"

void requestDhcp()
{

}

void receiveDhcp(void* data, unsigned int length)
{
    if (length < sizeof(DhcpPacket))
    {
        return;
    }
    DhcpPacket* pkt = (DhcpPacket*)data;
    if (pkt->op != 2) // we only care about replies
    {
        return;
    }
    unsigned int yiaddr = ntohl(pkt->yiaddr);
    unsigned int subnetMaskLocal = ntohl(*(unsigned int*)(pkt->sname)); //subnet mask is stored in sname for some reason
    unsigned int gatewayLocal = ntohl(*(unsigned int*)(pkt->sname + 4)); //gateway is stored in sname + 4 for some reason
    unsigned char* opt = (unsigned char*)pkt + sizeof(DhcpPacket);
    opt += 4; //skipping magic cookie
    while (*opt < 255)
    {
        unsigned char type = *opt++;

        if (type == 0) continue; // padding

        unsigned char len = *opt++;

        if (type == 1 && len == 4) // subnet mask
        {
            subnetMaskLocal = ntohl(*(unsigned int*)opt);
        }
        else if (type == 3 && len >= 4) // router (gateway)
        {
            gatewayLocal = ntohl(*(unsigned int*)opt);
        }

        opt += len;
    }
    if (yiaddr == 0)
    {
        printLineW("DHCP Request Failed: Default IP assigned");
        myIP = MY_IP; //default qemu ip
        gateway = QEMU_GATEWAY; //default qemu gateway
        subnetMask = SUBNET_MASK; //default subnet mask
        subnet = SUBNET; //default subnet
        return;
    }
    myIP = yiaddr;
    gateway = gatewayLocal;
    subnetMask = subnetMaskLocal;
    subnet = yiaddr & subnetMaskLocal;
    printLineW("DHCP Acquired!");
    printW("Assigned IP: ");
    printIP(yiaddr);
    printW("\nSubnet Mask: ");
    printIP(subnetMask);
    printW("\nGateway: ");
    printIP(gateway);
    printW("\nSubnet: ");
    printIP(subnet);
    printLineW("");
}

void discoverDhcp()
{
    char* buffer = (char*)kmalloc(548);
    memset(buffer, 0, 548);
    myIP = DHCP_SRC_IP;

    DhcpPacket* hdr = (DhcpPacket*)buffer;

    hdr->op = 1;
    hdr->htype = 1;
    hdr->hlen = 6;
    hdr->hops = 0;

    hdr->xid = htonl(randRange(0, 0xFFFF0000));

    hdr->secs = 0;
    hdr->flags = htons(0x8000);

    hdr->ciaddr = 0;
    hdr->yiaddr = 0;
    hdr->siaddr = 0;
    hdr->giaddr = 0;

    memcpy(hdr->chaddr, udpDev->mac, 6);

    unsigned char* opt = (unsigned char*)buffer + sizeof(DhcpPacket);

    *(unsigned int*)opt = htonl(0x63825363);
    opt += 4;

    *opt++ = 53; 
    *opt++ = 1;
    *opt++ = 1;

    *opt++ = 255;

    unsigned int size = opt - (unsigned char*)buffer;
    printLineW("Sending DHCP Discover...");
    udpSend(DHCP_SRC_IP, DHCP_BROADCAST_IP, DHCP_CLIENT_PORT, DHCP_SERVER_PORT, buffer, size);
}
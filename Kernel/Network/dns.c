#include "dns.h"

int dnsBuildQuery(unsigned char* buffer, const char* domain)
{
    DnsHeader* hdr = (DnsHeader*)buffer;
    hdr->id = htons(randRange(0x0, 0xFFFF));
    hdr->flags = htons(0x0100); //standard query with recursion desired
    hdr->qdcount = htons(1); //one question
    hdr->ancount = 0;
    hdr->nscount = 0;
    hdr->arcount = 0;
    unsigned char* qname = buffer + sizeof(DnsHeader);
    char* pos = domain;
    //ok so its a bit complicated not that much
    //the request is suppose to be like
    /*
    06 google 03 com 00
    where the first byte is the length of the label and then the label itself
    so we need to split the domain by dots and then write the length and the label
    and then write the qtype and qclass at the end
    */
    unsigned char* lenPos = qname; //so we will write the length here later
    unsigned char* labelStart = qname + 1; //the label starts after the length byte
    unsigned int labelLen = 0;
    while(*pos)
    {
        if(*pos == '.')
        {
            *lenPos = labelLen;
            lenPos = labelStart;
            labelLen = 0;
            labelStart = lenPos + 1;
            pos++;
        }
        else
        {
            *labelStart = *pos;
            labelStart++;
            labelLen++;
            pos++;
        }
    }
    *lenPos = labelLen;
    *labelStart = 0; //null byte to end the qname
    unsigned char* qtype = labelStart + 1; // right after the null byte
    *(unsigned short*)qtype = htons(1);  // QTYPE A
    *(unsigned short*)(qtype + 2) = htons(1);

    return qtype + 4 - buffer; // total length of the query

}

void dnsSendQuery(unsigned int srcIp, unsigned int dnsIp, const char* domain)
{
    unsigned char buffer[256];
    int queryLen = dnsBuildQuery(buffer, domain);
    udpSend(srcIp, dnsIp, EXAMPLE_PORT, DNS_PORT, buffer, queryLen);
}

void dnsReceive(void* data, unsigned int length)
{
    if (length < sizeof(DnsHeader))
    {
        return;
    }
    DnsHeader* hdr = (DnsHeader*)data;
    unsigned short qdcount = ntohs(hdr->qdcount);
    unsigned short ancount = ntohs(hdr->ancount);
    printLineW("DNS Response received!");
    unsigned char* ptr = (unsigned char*)data + sizeof(DnsHeader);
    for (int i = 0; i < qdcount; i++)
    {
        //skip the qname
        while (*ptr != 0)
        {
            ptr += (*ptr) + 1;
        }
        ptr++; // skip the null byte
        unsigned short qtype = ntohs(*(unsigned short*)ptr);
        ptr += 2;
        unsigned short qclass = ntohs(*(unsigned short*)ptr);
        ptr += 2;

    }
    for (int i = 0; i < ancount; i++)
    {
        // handle name (compressed or not)
        if ((*ptr & 0xC0) == 0xC0)
        {
            ptr += 2;
        }
        else
        {
            while (*ptr != 0)
            {
                ptr += (*ptr) + 1;
            }
            ptr++;
        }

        unsigned short atype = ntohs(*(unsigned short*)ptr);
        ptr += 2;

        unsigned short aclass = ntohs(*(unsigned short*)ptr);
        ptr += 2;

        unsigned int ttl = ntohl(*(unsigned int*)ptr);
        ptr += 4;

        unsigned short rdlength = ntohs(*(unsigned short*)ptr);
        ptr += 2;

        if (atype == 1 && aclass == 1 && rdlength == 4)
        {
            unsigned int ip = ntohl(*(unsigned int*)ptr);

            printW("Resolved IP: ");
            printIP(ip);
            printLineW("");
        }

        ptr += rdlength;
    }
}
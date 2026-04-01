#ifndef DNS_H
#define DNS_H

#include "udp.h"

//starting to comment ill start with this:
/*
DNS
- DNS Header Structure:
    - id: unique identifier for the query/response.
    - flags: query/response flags (e.g., recursion desired).
    - qdcount: number of questions. - how many queries are in the question section.
    - ancount: number of answers. - how many answers are in the answer section.
    - nscount: number of authority records. - which servers are authoritative for the domain.
    - arcount: number of additional records. - extra info like IP addresses for name servers.

- DNS Query Format:
    - Question Section:
        - qname: domain name in a specific format (labels prefixed by length).
        - qtype: type of query (e.g., A for IPv4 address).
        - qclass: class of query (usually IN for internet). others exist but IN is most common.
*/

#define DNS_PORT 53
#define DNS_SERVER 0x08080808 //Google Public DNS IP in hex
#define EXAMPLE_PORT 12345

typedef struct DnsHeader 
{
    unsigned short id;
    unsigned short flags;
    unsigned short qdcount;  
    unsigned short ancount;
    unsigned short nscount;
    unsigned short arcount;
} __attribute__((packed)) DnsHeader;

int dnsBuildQuery(unsigned char* buffer, const char* domain);
void dnsSendQuery(unsigned int srcIp, unsigned int dnsIp, const char* domain);
void dnsReceive(void* data, unsigned int length);
#endif
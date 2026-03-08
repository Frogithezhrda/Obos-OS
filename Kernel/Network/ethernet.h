#ifndef ETHERNET_H
#define ETHERNET_H


#define ETHERNET_HEADER_SIZE 14

typedef struct
{
    unsigned char dest[6];
    unsigned char src[6];
    unsigned short type;
} EthernetHeader;

void ethernetInit();
void ethernetSend(void* data, unsigned int length);
void ethernetReceive(void* buffer, unsigned int length);

#endif // ETHERNET_H
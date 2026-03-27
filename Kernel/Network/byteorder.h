#ifndef BYTEORDER_H
#define BYTEORDER_H

static inline unsigned short ntohs(unsigned short val)
{
    return (val << 8) | (val >> 8);
}

static inline unsigned short htons(unsigned short val)
{
    return ntohs(val);
}

static inline unsigned int ntohl(unsigned int val)
{
    return ((unsigned int)ntohs(val & 0xFFFF) << 16) | ntohs(val >> 16);
}

static inline unsigned int htonl(unsigned int val)
{
    return ntohl(val);
}

#endif // BYTEORDER_H
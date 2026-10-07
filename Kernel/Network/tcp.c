#include "tcp.h"

enum { tcpMss = 1400, tcpWindow = 4096, synRetryTicks = 50 };

typedef struct
{
    TcpState state;
    unsigned int remoteIp;
    unsigned short localPort;
    unsigned short remotePort;
    unsigned int sndUna;
    unsigned int sndNxt;
    unsigned int rcvNxt;
    unsigned int ticks;
    unsigned int retries;
    volatile unsigned char ackPending;
    volatile unsigned char closePending;
    TcpDataCb onData;
    TcpCloseCb onClose;
} TcpConn;

static NetDevice* tcpDev;
static TcpConn conn;
static unsigned int isnCounter = 0x1000;

void tcpInit(NetDevice* dev)
{
    tcpDev = dev;
    conn.state = TCP_CLOSED;
}

TcpState tcpGetState()
{
    return conn.state;
}

static unsigned int sumBytes(unsigned int sum, const unsigned char* p, unsigned int len)
{
    while (len > 1)
    {
        sum += (p[0] << 8) | p[1];
        p += 2;
        len -= 2;
    }
    if (len)
    {
        sum += p[0] << 8;
    }
    return sum;
}

static unsigned short tcpChecksum(unsigned int srcIp, unsigned int dstIp, const unsigned char* seg, unsigned int len)
{
    unsigned int sum = 0;
    sum += (srcIp >> 16) + (srcIp & 0xFFFF);
    sum += (dstIp >> 16) + (dstIp & 0xFFFF);
    sum += TCP_PROTOCOL;
    sum += len;
    sum = sumBytes(sum, seg, len);
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (unsigned short)~sum;
}

static void sendSegment(unsigned char flags, unsigned int seq, const void* data, unsigned int len)
{
    unsigned char seg[sizeof(TcpHeader) + tcpMss];
    if (len > tcpMss)
    {
        return;
    }
    TcpHeader* h = (TcpHeader*)seg;
    h->srcPort = htons(conn.localPort);
    h->destPort = htons(conn.remotePort);
    h->seqNum = htonl(seq);
    h->ackNum = htonl((flags & TCP_ACK) ? conn.rcvNxt : 0);
    h->dataOffset = (sizeof(TcpHeader) / 4) << 4;
    h->flags = flags;
    h->windowSize = htons(tcpWindow);
    h->checksum = 0;
    h->urgentPointer = 0;
    const unsigned char* src = (const unsigned char*)data;
    for (unsigned int i = 0; i < len; i++)
    {
        seg[sizeof(TcpHeader) + i] = src[i];
    }
    unsigned int total = sizeof(TcpHeader) + len;
    h->checksum = htons(tcpChecksum(myIP, conn.remoteIp, seg, total));
    ipSend(conn.remoteIp, TCP_PROTOCOL, seg, total);
}

int tcpConnect(unsigned int dstIp, unsigned short dstPort, TcpDataCb onData, TcpCloseCb onClose)
{
    if (conn.state != TCP_CLOSED)
    {
        return ERROR;
    }
    conn.remoteIp = dstIp;
    conn.remotePort = dstPort;
    conn.localPort = 49152 + (isnCounter & 0x3FFF);
    isnCounter += 64000;
    conn.sndUna = isnCounter;
    conn.sndNxt = isnCounter + 1;
    conn.rcvNxt = 0;
    conn.ticks = 0;
    conn.retries = 0;
    conn.ackPending = 0;
    conn.closePending = 0;
    conn.onData = onData;
    conn.onClose = onClose;
    conn.state = TCP_SYN_SENT;
    sendSegment(TCP_SYN, conn.sndUna, 0, 0);
    return SUCCESS;
}

int tcpSend(void* data, unsigned int length)
{
    if (conn.state != TCP_ESTABLISHED)
    {
        return ERROR;
    }
    unsigned char* p = (unsigned char*)data;
    while (length)
    {
        unsigned int chunk = length > tcpMss ? tcpMss : length;
        unsigned int seq = conn.sndNxt;
        conn.sndNxt += chunk;
        sendSegment(TCP_PSH | TCP_ACK, seq, p, chunk);
        p += chunk;
        length -= chunk;
    }
    return SUCCESS;
}

void tcpClose()
{
    if (conn.state == TCP_ESTABLISHED || conn.state == TCP_CLOSE_WAIT)
    {
        unsigned int seq = conn.sndNxt;
        conn.sndNxt++;
        conn.state = (conn.state == TCP_ESTABLISHED) ? TCP_FIN_WAIT_1 : TCP_LAST_ACK;
        sendSegment(TCP_FIN | TCP_ACK, seq, 0, 0);
    }
}

void tcpTick()
{
    if (conn.closePending)
    {
        conn.closePending = 0;
        conn.ackPending = 0;
        tcpClose();
    }
    else if (conn.ackPending)
    {
        conn.ackPending = 0;
        sendSegment(TCP_ACK, conn.sndNxt, 0, 0);
    }
    if (conn.state == TCP_SYN_SENT && ++conn.ticks >= synRetryTicks)
    {
        conn.ticks = 0;
        if (++conn.retries > 5)
        {
            conn.state = TCP_CLOSED;
            return;
        }
        sendSegment(TCP_SYN, conn.sndUna, 0, 0);
    }
}

void tcpReceive(NetDevice* dev, const void* buffer, unsigned int length)
{
    (void)dev;
    if (length < sizeof(TcpHeader) || conn.state == TCP_CLOSED)
    {
        return;
    }
    if (tcpChecksum(conn.remoteIp, myIP, (const unsigned char*)buffer, length) != 0)
    {
        return;
    }
    const TcpHeader* h = (const TcpHeader*)buffer;
    if (ntohs(h->destPort) != conn.localPort || ntohs(h->srcPort) != conn.remotePort)
    {
        return;
    }
    unsigned int hdrLen = (h->dataOffset >> 4) * 4;
    if (hdrLen < sizeof(TcpHeader) || hdrLen > length)
    {
        return;
    }
    unsigned char flags = h->flags;
    unsigned int seq = ntohl(h->seqNum);
    unsigned int ack = ntohl(h->ackNum);
    const unsigned char* data = (const unsigned char*)buffer + hdrLen;
    unsigned int dataLen = length - hdrLen;

    if (flags & TCP_RST)
    {
        conn.state = TCP_CLOSED;
        if (conn.onClose)
        {
            conn.onClose();
        }
        return;
    }

    if (conn.state == TCP_SYN_SENT)
    {
        if ((flags & (TCP_SYN | TCP_ACK)) == (TCP_SYN | TCP_ACK) && ack == conn.sndNxt)
        {
            conn.sndUna = ack;
            conn.rcvNxt = seq + 1;
            conn.ackPending = 1;
            conn.state = TCP_ESTABLISHED;
        }
        return;
    }

    if (flags & TCP_SYN)
    {
        conn.ackPending = 1;
        return;
    }

    if (flags & TCP_ACK)
    {
        conn.sndUna = ack;
        if (conn.state == TCP_FIN_WAIT_1 && ack == conn.sndNxt)
        {
            conn.state = TCP_FIN_WAIT_2;
        }
        if (conn.state == TCP_LAST_ACK && ack == conn.sndNxt)
        {
            conn.state = TCP_CLOSED;
            if (conn.onClose)
            {
                conn.onClose();
            }
            return;
        }
    }

    if (dataLen > 0)
    {
        int inOrder = (seq == conn.rcvNxt);
        if (inOrder)
        {
            conn.rcvNxt += dataLen;
            if (conn.onData)
            {
                conn.onData(data, dataLen);
            }
        }
        conn.ackPending = 1;
        if (!inOrder)
        {
            return;
        }
    }

    if ((flags & TCP_FIN) && seq + dataLen == conn.rcvNxt)
    {
        conn.rcvNxt++;
        conn.ackPending = 1;
        if (conn.state == TCP_ESTABLISHED)
        {
            conn.state = TCP_CLOSE_WAIT;
            conn.closePending = 1;
            if (conn.onClose)
            {
                conn.onClose();
            }
        }
        else if (conn.state == TCP_FIN_WAIT_1 || conn.state == TCP_FIN_WAIT_2)
        {
            conn.state = TCP_CLOSED;
            if (conn.onClose)
            {
                conn.onClose();
            }
        }
    }
}
void tcpAbort()
{
    conn.state = TCP_CLOSED;
    conn.ackPending = 0;
    conn.closePending = 0;
}
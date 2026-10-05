#include "tcp.h"
 
#define HTTP_PORT 80
 
static unsigned int appendStr(char* buf, unsigned int pos, unsigned int max, const char* s)
{
    while (*s && pos < max - 1)
    {
        buf[pos++] = *s++;
    }
    buf[pos] = '\0';
    return pos;
}
 
static unsigned int buildGet(char* buf, unsigned int max, const char* host, const char* path)
{
    unsigned int pos = 0;
    pos = appendStr(buf, pos, max, "GET ");
    pos = appendStr(buf, pos, max, path);
    pos = appendStr(buf, pos, max, " HTTP/1.1\r\nHost: ");
    pos = appendStr(buf, pos, max, host);
    pos = appendStr(buf, pos, max, "\r\nUser-Agent: obos/0.1\r\nAccept: */*\r\nConnection: close\r\n\r\n");
    return pos;
}
 
static void onHttpData(const unsigned char* data, unsigned int len)
{
    for (unsigned int i = 0; i < len; i++)
    {
        printCharW(data[i]);
    }
}
 
static volatile int httpDone = 0;
 
static void onHttpClose()
{
    httpDone = 1;
}
 
int httpGet(unsigned int ip, const char* host, const char* path)
{
    char request[256];
    unsigned int len = buildGet(request, sizeof(request), host, path);
    httpDone = 0;
    if (tcpConnect(ip, HTTP_PORT, onHttpData, onHttpClose) != SUCCESS)
    {
        return ERROR;
    }
    while (tcpGetState() == TCP_SYN_SENT)
    {
        tcpTick();   // call this from a timer instead if your loop spins too fast
    }
    if (tcpGetState() != TCP_ESTABLISHED)
    {
        printLineW("http: connect failed");
        return ERROR;
    }
    tcpSend(request, len);
    while (!httpDone)
    {
        tcpTick();
    }
    tcpTick();
    return SUCCESS;
}
 

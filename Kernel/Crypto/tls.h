#ifndef TLS_H
#define TLS_H

#include "tlsCrypto.h"

enum
{
    tlsRecMax = 16384 + 2048,
    tlsHsMax = 20480,
    tlsOutMax = 5 + 8 + 16384 + 16
};

typedef void (*TlsSendFn)(void* user, const unsigned char* data, unsigned int len);
typedef void (*TlsDataFn)(void* user, const unsigned char* data, unsigned int len);

typedef enum
{
    tlsIdle,
    tlsHandshake,
    tlsReady,
    tlsClosed,
    tlsFailed
} TlsState;

typedef struct
{
    TlsState state;
    const char* err;
    int alertCode;
    TlsSendFn send;
    TlsDataFn onData;
    void* user;
    char host[64];
    unsigned char clientRandom[32];
    unsigned char serverRandom[32];
    unsigned char priv[32];
    unsigned char pub[32];
    unsigned char serverPub[32];
    unsigned char master[48];
    unsigned int suite;
    int gotHello;
    int gotKeyExchange;
    int certRequested;
    int sentFinished;
    int verified;
    Sha256 hash;
    Gcm wr;
    Gcm rd;
    unsigned char wrIv[4];
    unsigned char rdIv[4];
    unsigned int wrSeqHi;
    unsigned int wrSeqLo;
    unsigned int rdSeqHi;
    unsigned int rdSeqLo;
    int encOut;
    int encIn;
    int recMinor;
    unsigned int recLen;
    unsigned int hsLen;
    unsigned char rec[5 + tlsRecMax];
    unsigned char hs[tlsHsMax];
    unsigned char out[tlsOutMax];
} TlsConn;

void tlsInit(TlsConn* t, const char* host, const unsigned char random[64], TlsSendFn send, TlsDataFn onData, void* user);
void tlsStart(TlsConn* t);
void tlsFeed(TlsConn* t, const unsigned char* data, unsigned int len);
int tlsWrite(TlsConn* t, const void* data, unsigned int len);
void tlsClose(TlsConn* t);
TlsState tlsGetState(const TlsConn* t);
const char* tlsErrorText(const TlsConn* t);

#endif
#include "tls.h"

enum
{
    ctCcs = 20,
    ctAlert = 21,
    ctHandshake = 22,
    ctAppData = 23
};

enum
{
    hsClientHello = 1,
    hsServerHello = 2,
    hsCertificate = 11,
    hsServerKeyExchange = 12,
    hsCertRequest = 13,
    hsServerHelloDone = 14,
    hsClientKeyExchange = 16,
    hsFinished = 20
};

enum
{
    suiteEcdheRsaGcm128 = 0xC02F,
    suiteEcdheEcdsaGcm128 = 0xC02B
};

static void fail(TlsConn* t, const char* msg)
{
    if (t->state != tlsFailed)
    {
        t->state = tlsFailed;
        t->err = msg;
    }
}

static void put16(unsigned char* p, unsigned int v)
{
    p[0] = v >> 8;
    p[1] = v;
}

static void seqBytes(unsigned char* out, unsigned int hi, unsigned int lo)
{
    out[0] = hi >> 24;
    out[1] = hi >> 16;
    out[2] = hi >> 8;
    out[3] = hi;
    out[4] = lo >> 24;
    out[5] = lo >> 16;
    out[6] = lo >> 8;
    out[7] = lo;
}

static void seqNext(unsigned int* hi, unsigned int* lo)
{
    *lo += 1;
    if (*lo == 0)
    {
        *hi += 1;
    }
}

static void prf(unsigned char* out, unsigned int outLen, const unsigned char* secret, unsigned int secretLen, const char* label, const unsigned char* seed, unsigned int seedLen)
{
    unsigned char ls[96];
    unsigned char a[32];
    unsigned char block[32];
    unsigned int labelLen = 0;
    while (label[labelLen])
    {
        ls[labelLen] = label[labelLen];
        labelLen++;
    }
    tlsCopy(ls + labelLen, seed, seedLen);
    hmacSha256(a, secret, secretLen, ls, labelLen + seedLen, 0, 0);
    while (outLen > 0)
    {
        unsigned int n = outLen > 32 ? 32 : outLen;
        hmacSha256(block, secret, secretLen, a, 32, ls, labelLen + seedLen);
        tlsCopy(out, block, n);
        out += n;
        outLen -= n;
        hmacSha256(a, secret, secretLen, a, 32, 0, 0);
    }
}

static void sendRecord(TlsConn* t, unsigned char type, const unsigned char* data, unsigned int len)
{
    unsigned char* o = t->out;
    o[0] = type;
    o[1] = 3;
    o[2] = t->recMinor;
    if (!t->encOut)
    {
        put16(o + 3, len);
        tlsCopy(o + 5, data, len);
        t->send(t->user, o, 5 + len);
        return;
    }
    {
        unsigned char nonce[12];
        unsigned char aad[13];
        unsigned char seq[8];
        seqBytes(seq, t->wrSeqHi, t->wrSeqLo);
        tlsCopy(nonce, t->wrIv, 4);
        tlsCopy(nonce + 4, seq, 8);
        tlsCopy(aad, seq, 8);
        aad[8] = type;
        aad[9] = 3;
        aad[10] = 3;
        put16(aad + 11, len);
        put16(o + 3, 8 + len + 16);
        tlsCopy(o + 5, seq, 8);
        gcmSeal(&t->wr, nonce, aad, 13, data, len, o + 13, o + 13 + len);
        seqNext(&t->wrSeqHi, &t->wrSeqLo);
        t->send(t->user, o, 13 + len + 16);
    }
}

static void sendHandshake(TlsConn* t, unsigned char type, const unsigned char* body, unsigned int len)
{
    unsigned char msg[600];
    if (len + 4 > sizeof(msg))
    {
        fail(t, "handshake message too large to send");
        return;
    }
    msg[0] = type;
    msg[1] = len >> 16;
    msg[2] = len >> 8;
    msg[3] = len;
    tlsCopy(msg + 4, body, len);
    sha256Update(&t->hash, msg, len + 4);
    sendRecord(t, ctHandshake, msg, len + 4);
}

static int looksLikeName(const char* host)
{
    for (int i = 0; host[i]; i++)
    {
        if ((host[i] < '0' || host[i] > '9') && host[i] != '.')
        {
            return 1;
        }
    }
    return 0;
}

void tlsInit(TlsConn* t, const char* host, const unsigned char random[64], TlsSendFn send, TlsDataFn onData, void* user)
{
    unsigned int n = 0;
    tlsZero(t, sizeof(*t));
    t->state = tlsIdle;
    t->send = send;
    t->onData = onData;
    t->user = user;
    while (host[n] && n < sizeof(t->host) - 1)
    {
        t->host[n] = host[n];
        n++;
    }
    tlsCopy(t->clientRandom, random, 32);
    tlsCopy(t->priv, random + 32, 32);
    x25519Base(t->pub, t->priv);
    sha256Init(&t->hash);
    t->recMinor = 1;
}

void tlsStart(TlsConn* t)
{
    unsigned char b[512];
    unsigned int n = 0;
    unsigned int extStart;
    unsigned int hostLen = 0;
    if (t->state != tlsIdle)
    {
        return;
    }
    while (t->host[hostLen])
    {
        hostLen++;
    }
    b[n++] = 3;
    b[n++] = 3;
    tlsCopy(b + n, t->clientRandom, 32);
    n += 32;
    b[n++] = 0;
    put16(b + n, 6);
    n += 2;
    put16(b + n, suiteEcdheRsaGcm128);
    n += 2;
    put16(b + n, suiteEcdheEcdsaGcm128);
    n += 2;
    put16(b + n, 0x00FF);
    n += 2;
    b[n++] = 1;
    b[n++] = 0;
    extStart = n;
    n += 2;
    if (hostLen > 0 && looksLikeName(t->host))
    {
        put16(b + n, 0);
        put16(b + n + 2, hostLen + 5);
        put16(b + n + 4, hostLen + 3);
        b[n + 6] = 0;
        put16(b + n + 7, hostLen);
        tlsCopy(b + n + 9, t->host, hostLen);
        n += 9 + hostLen;
    }
    put16(b + n, 10);
    put16(b + n + 2, 6);
    put16(b + n + 4, 4);
    put16(b + n + 6, 0x001d);
    put16(b + n + 8, 0x0017);
    n += 10;
    put16(b + n, 11);
    put16(b + n + 2, 2);
    b[n + 4] = 1;
    b[n + 5] = 0;
    n += 6;
    put16(b + n, 13);
    put16(b + n + 2, 6);
    put16(b + n + 4, 4);
    put16(b + n + 6, 0x0401);
    put16(b + n + 8, 0x0403);
    n += 10;
    put16(b + extStart, n - extStart - 2);
    t->state = tlsHandshake;
    sendHandshake(t, hsClientHello, b, n);
    t->recMinor = 3;
}

static void handleServerHello(TlsConn* t, const unsigned char* b, unsigned int len)
{
    unsigned int sidLen;
    unsigned int pos;
    if (t->gotHello || len < 38 || b[0] != 3 || b[1] != 3)
    {
        fail(t, "bad ServerHello");
        return;
    }
    tlsCopy(t->serverRandom, b + 2, 32);
    sidLen = b[34];
    pos = 35 + sidLen;
    if (pos + 3 > len)
    {
        fail(t, "bad ServerHello");
        return;
    }
    t->suite = (b[pos] << 8) | b[pos + 1];
    if (t->suite != suiteEcdheRsaGcm128 && t->suite != suiteEcdheEcdsaGcm128)
    {
        fail(t, "server chose an unsupported cipher suite");
        return;
    }
    if (b[pos + 2] != 0)
    {
        fail(t, "server chose compression");
        return;
    }
    t->gotHello = 1;
}

static void handleKeyExchange(TlsConn* t, const unsigned char* b, unsigned int len)
{
    if (!t->gotHello || len < 40 || b[0] != 3 || b[1] != 0 || b[2] != 0x1d || b[3] != 32)
    {
        fail(t, "unsupported key exchange (need x25519)");
        return;
    }
    tlsCopy(t->serverPub, b + 4, 32);
    t->gotKeyExchange = 1;
}

static void deriveKeys(TlsConn* t, const unsigned char* shared)
{
    unsigned char seed[64];
    unsigned char block[40];
    tlsCopy(seed, t->clientRandom, 32);
    tlsCopy(seed + 32, t->serverRandom, 32);
    prf(t->master, 48, shared, 32, "master secret", seed, 64);
    tlsCopy(seed, t->serverRandom, 32);
    tlsCopy(seed + 32, t->clientRandom, 32);
    prf(block, 40, t->master, 48, "key expansion", seed, 64);
    gcmInit(&t->wr, block);
    gcmInit(&t->rd, block + 16);
    tlsCopy(t->wrIv, block + 32, 4);
    tlsCopy(t->rdIv, block + 36, 4);
    tlsZero(block, sizeof(block));
    tlsZero(seed, sizeof(seed));
}

static void finishedData(TlsConn* t, const char* label, unsigned char out[12])
{
    Sha256 copy;
    unsigned char digest[32];
    tlsCopy(&copy, &t->hash, sizeof(copy));
    sha256Final(&copy, digest);
    prf(out, 12, t->master, 48, label, digest, 32);
}

static void handleHelloDone(TlsConn* t, unsigned int len)
{
    unsigned char shared[32];
    unsigned char zero = 0;
    unsigned char cke[33];
    unsigned char verify[12];
    unsigned char ccs = 1;
    if (len != 0 || !t->gotHello || !t->gotKeyExchange)
    {
        fail(t, "unexpected ServerHelloDone");
        return;
    }
    if (t->certRequested)
    {
        unsigned char empty[3] = {0, 0, 0};
        sendHandshake(t, hsCertificate, empty, 3);
    }
    x25519(shared, t->priv, t->serverPub);
    for (int i = 0; i < 32; i++)
    {
        zero |= shared[i];
    }
    if (zero == 0)
    {
        fail(t, "bad server key share");
        return;
    }
    deriveKeys(t, shared);
    tlsZero(shared, sizeof(shared));
    cke[0] = 32;
    tlsCopy(cke + 1, t->pub, 32);
    sendHandshake(t, hsClientKeyExchange, cke, 33);
    sendRecord(t, ctCcs, &ccs, 1);
    t->encOut = 1;
    finishedData(t, "client finished", verify);
    sendHandshake(t, hsFinished, verify, 12);
    t->sentFinished = 1;
}

static void handleFinished(TlsConn* t, const unsigned char* b, unsigned int len)
{
    unsigned char expect[12];
    if (!t->sentFinished || !t->encIn || len != 12)
    {
        fail(t, "unexpected Finished");
        return;
    }
    finishedData(t, "server finished", expect);
    if (!tlsConstEq(expect, b, 12))
    {
        fail(t, "server Finished does not match");
        return;
    }
    t->state = tlsReady;
}

static void processHandshake(TlsConn* t)
{
    while (t->hsLen >= 4 && t->state == tlsHandshake)
    {
        unsigned int type = t->hs[0];
        unsigned int mlen = ((unsigned int)t->hs[1] << 16) | ((unsigned int)t->hs[2] << 8) | t->hs[3];
        if (mlen + 4 > tlsHsMax)
        {
            fail(t, "handshake message too large");
            return;
        }
        if (t->hsLen < mlen + 4)
        {
            return;
        }
        if (type != hsFinished)
        {
            sha256Update(&t->hash, t->hs, mlen + 4);
        }
        if (type == hsServerHello)
        {
            handleServerHello(t, t->hs + 4, mlen);
        }
        else if (type == hsServerKeyExchange)
        {
            handleKeyExchange(t, t->hs + 4, mlen);
        }
        else if (type == hsCertRequest)
        {
            t->certRequested = 1;
        }
        else if (type == hsServerHelloDone)
        {
            handleHelloDone(t, mlen);
        }
        else if (type == hsFinished)
        {
            handleFinished(t, t->hs + 4, mlen);
        }
        tlsMove(t->hs, t->hs + mlen + 4, t->hsLen - (mlen + 4));
        t->hsLen -= mlen + 4;
    }
}

static void processRecord(TlsConn* t)
{
    unsigned char type = t->rec[0];
    unsigned char* body = t->rec + 5;
    unsigned int len = ((unsigned int)t->rec[3] << 8) | t->rec[4];
    if (t->rec[1] != 3)
    {
        fail(t, "bad record version");
        return;
    }
    if (t->encIn)
    {
        unsigned char nonce[12];
        unsigned char aad[13];
        unsigned int plain;
        if (len < 24)
        {
            fail(t, "short encrypted record");
            return;
        }
        plain = len - 24;
        tlsCopy(nonce, t->rdIv, 4);
        tlsCopy(nonce + 4, body, 8);
        seqBytes(aad, t->rdSeqHi, t->rdSeqLo);
        aad[8] = type;
        aad[9] = 3;
        aad[10] = 3;
        put16(aad + 11, plain);
        if (gcmOpen(&t->rd, nonce, aad, 13, body + 8, plain, body + 8 + plain, body) != 0)
        {
            fail(t, "bad record MAC");
            return;
        }
        seqNext(&t->rdSeqHi, &t->rdSeqLo);
        len = plain;
    }
    if (type == ctCcs)
    {
        if (t->encIn || !t->sentFinished || len != 1 || body[0] != 1)
        {
            fail(t, "unexpected ChangeCipherSpec");
            return;
        }
        t->encIn = 1;
    }
    else if (type == ctAlert)
    {
        if (len >= 2)
        {
            t->alertCode = body[1];
            if (body[1] == 0)
            {
                t->state = tlsClosed;
            }
            else if (body[0] == 2)
            {
                fail(t, "server sent a fatal alert");
            }
        }
    }
    else if (type == ctHandshake)
    {
        if (t->state != tlsHandshake)
        {
            fail(t, "unexpected handshake data");
            return;
        }
        if (t->hsLen + len > tlsHsMax)
        {
            fail(t, "handshake message too large");
            return;
        }
        tlsCopy(t->hs + t->hsLen, body, len);
        t->hsLen += len;
        processHandshake(t);
    }
    else if (type == ctAppData)
    {
        if (t->state != tlsReady)
        {
            fail(t, "application data before handshake end");
            return;
        }
        if (len > 0 && t->onData)
        {
            t->onData(t->user, body, len);
        }
    }
    else
    {
        fail(t, "unknown record type");
    }
}

void tlsFeed(TlsConn* t, const unsigned char* data, unsigned int len)
{
    while (len > 0 && t->state != tlsFailed && t->state != tlsClosed)
    {
        unsigned int need;
        unsigned int n;
        if (t->recLen < 5)
        {
            need = 5 - t->recLen;
        }
        else
        {
            need = 5 + (((unsigned int)t->rec[3] << 8) | t->rec[4]) - t->recLen;
        }
        n = len < need ? len : need;
        tlsCopy(t->rec + t->recLen, data, n);
        t->recLen += n;
        data += n;
        len -= n;
        if (t->recLen >= 5)
        {
            unsigned int rl = ((unsigned int)t->rec[3] << 8) | t->rec[4];
            if (rl > tlsRecMax)
            {
                fail(t, "record too large");
                return;
            }
            if (t->recLen == 5 + rl)
            {
                processRecord(t);
                t->recLen = 0;
            }
        }
    }
}

int tlsWrite(TlsConn* t, const void* data, unsigned int len)
{
    const unsigned char* p = (const unsigned char*)data;
    if (t->state != tlsReady)
    {
        return -1;
    }
    while (len > 0)
    {
        unsigned int n = len > 16384 ? 16384 : len;
        sendRecord(t, ctAppData, p, n);
        p += n;
        len -= n;
    }
    return 0;
}

void tlsClose(TlsConn* t)
{
    unsigned char alert[2] = {1, 0};
    if (t->state == tlsReady)
    {
        sendRecord(t, ctAlert, alert, 2);
        t->state = tlsClosed;
    }
}

TlsState tlsGetState(const TlsConn* t)
{
    return t->state;
}

const char* tlsErrorText(const TlsConn* t)
{
    return t->err ? t->err : "";
}
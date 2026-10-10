#include "tlsCrypto.h"

void tlsCopy(void* dst, const void* src, unsigned int len)
{
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    for (unsigned int i = 0; i < len; i++)
    {
        d[i] = s[i];
    }
}

void tlsMove(void* dst, const void* src, unsigned int len)
{
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    if (d < s)
    {
        for (unsigned int i = 0; i < len; i++)
        {
            d[i] = s[i];
        }
    }
    else
    {
        for (unsigned int i = len; i > 0; i--)
        {
            d[i - 1] = s[i - 1];
        }
    }
}

void tlsZero(void* dst, unsigned int len)
{
    volatile unsigned char* d = (volatile unsigned char*)dst;
    for (unsigned int i = 0; i < len; i++)
    {
        d[i] = 0;
    }
}

int tlsConstEq(const unsigned char* a, const unsigned char* b, unsigned int len)
{
    unsigned char diff = 0;
    for (unsigned int i = 0; i < len; i++)
    {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

static const unsigned int k256[64] =
{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void sha256Block(Sha256* c, const unsigned char* p)
{
    unsigned int w[64];
    unsigned int a = c->h[0];
    unsigned int b = c->h[1];
    unsigned int cc = c->h[2];
    unsigned int d = c->h[3];
    unsigned int e = c->h[4];
    unsigned int f = c->h[5];
    unsigned int g = c->h[6];
    unsigned int h = c->h[7];
    for (int i = 0; i < 16; i++)
    {
        w[i] = ((unsigned int)p[4 * i] << 24) | ((unsigned int)p[4 * i + 1] << 16) | ((unsigned int)p[4 * i + 2] << 8) | p[4 * i + 3];
    }
    for (int i = 16; i < 64; i++)
    {
        unsigned int s0 = ROTR(w[i - 15], 7) ^ ROTR(w[i - 15], 18) ^ (w[i - 15] >> 3);
        unsigned int s1 = ROTR(w[i - 2], 17) ^ ROTR(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    for (int i = 0; i < 64; i++)
    {
        unsigned int s1 = ROTR(e, 6) ^ ROTR(e, 11) ^ ROTR(e, 25);
        unsigned int ch = (e & f) ^ (~e & g);
        unsigned int t1 = h + s1 + ch + k256[i] + w[i];
        unsigned int s0 = ROTR(a, 2) ^ ROTR(a, 13) ^ ROTR(a, 22);
        unsigned int maj = (a & b) ^ (a & cc) ^ (b & cc);
        unsigned int t2 = s0 + maj;
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = cc;
        cc = b;
        b = a;
        a = t1 + t2;
    }
    c->h[0] += a;
    c->h[1] += b;
    c->h[2] += cc;
    c->h[3] += d;
    c->h[4] += e;
    c->h[5] += f;
    c->h[6] += g;
    c->h[7] += h;
}

void sha256Init(Sha256* c)
{
    c->h[0] = 0x6a09e667;
    c->h[1] = 0xbb67ae85;
    c->h[2] = 0x3c6ef372;
    c->h[3] = 0xa54ff53a;
    c->h[4] = 0x510e527f;
    c->h[5] = 0x9b05688c;
    c->h[6] = 0x1f83d9ab;
    c->h[7] = 0x5be0cd19;
    c->bufLen = 0;
    c->lenLo = 0;
    c->lenHi = 0;
}

void sha256Update(Sha256* c, const void* data, unsigned int len)
{
    const unsigned char* p = (const unsigned char*)data;
    unsigned int old = c->lenLo;
    c->lenLo += len;
    if (c->lenLo < old)
    {
        c->lenHi++;
    }
    while (len > 0)
    {
        unsigned int n = 64 - c->bufLen;
        if (n > len)
        {
            n = len;
        }
        tlsCopy(c->buf + c->bufLen, p, n);
        c->bufLen += n;
        p += n;
        len -= n;
        if (c->bufLen == 64)
        {
            sha256Block(c, c->buf);
            c->bufLen = 0;
        }
    }
}

void sha256Final(Sha256* c, unsigned char out[32])
{
    unsigned int bitsHi = (c->lenHi << 3) | (c->lenLo >> 29);
    unsigned int bitsLo = c->lenLo << 3;
    unsigned char pad[72];
    unsigned int padLen = (c->bufLen < 56) ? (56 - c->bufLen) : (120 - c->bufLen);
    tlsZero(pad, sizeof(pad));
    pad[0] = 0x80;
    sha256Update(c, pad, padLen);
    pad[0] = bitsHi >> 24;
    pad[1] = bitsHi >> 16;
    pad[2] = bitsHi >> 8;
    pad[3] = bitsHi;
    pad[4] = bitsLo >> 24;
    pad[5] = bitsLo >> 16;
    pad[6] = bitsLo >> 8;
    pad[7] = bitsLo;
    sha256Update(c, pad, 8);
    for (int i = 0; i < 8; i++)
    {
        out[4 * i] = c->h[i] >> 24;
        out[4 * i + 1] = c->h[i] >> 16;
        out[4 * i + 2] = c->h[i] >> 8;
        out[4 * i + 3] = c->h[i];
    }
}

void hmacSha256(unsigned char out[32], const unsigned char* key, unsigned int keyLen, const unsigned char* a, unsigned int aLen, const unsigned char* b, unsigned int bLen)
{
    unsigned char k[64];
    unsigned char pad[64];
    unsigned char inner[32];
    Sha256 c;
    tlsZero(k, sizeof(k));
    if (keyLen > 64)
    {
        sha256Init(&c);
        sha256Update(&c, key, keyLen);
        sha256Final(&c, k);
    }
    else
    {
        tlsCopy(k, key, keyLen);
    }
    for (int i = 0; i < 64; i++)
    {
        pad[i] = k[i] ^ 0x36;
    }
    sha256Init(&c);
    sha256Update(&c, pad, 64);
    sha256Update(&c, a, aLen);
    sha256Update(&c, b, bLen);
    sha256Final(&c, inner);
    for (int i = 0; i < 64; i++)
    {
        pad[i] = k[i] ^ 0x5c;
    }
    sha256Init(&c);
    sha256Update(&c, pad, 64);
    sha256Update(&c, inner, 32);
    sha256Final(&c, out);
    tlsZero(k, sizeof(k));
}

static unsigned char sbox[256];
static int sboxReady = 0;

static unsigned char xtime(unsigned char x)
{
    return (unsigned char)((x << 1) ^ ((x & 0x80) ? 0x1b : 0));
}

static unsigned char rotl8(unsigned char x, int s)
{
    return (unsigned char)((x << s) | (x >> (8 - s)));
}

static void sboxInit()
{
    unsigned char p = 1;
    unsigned char q = 1;
    do
    {
        unsigned char x;
        p = (unsigned char)(p ^ xtime(p));
        q ^= (unsigned char)(q << 1);
        q ^= (unsigned char)(q << 2);
        q ^= (unsigned char)(q << 4);
        if (q & 0x80)
        {
            q ^= 0x09;
        }
        x = (unsigned char)(q ^ rotl8(q, 1) ^ rotl8(q, 2) ^ rotl8(q, 3) ^ rotl8(q, 4));
        sbox[p] = (unsigned char)(x ^ 0x63);
    } while (p != 1);
    sbox[0] = 0x63;
    sboxReady = 1;
}

void aes128SetKey(Aes128* a, const unsigned char key[16])
{
    unsigned char rcon = 1;
    if (!sboxReady)
    {
        sboxInit();
    }
    tlsCopy(a->rk, key, 16);
    for (int i = 4; i < 44; i++)
    {
        unsigned char t[4];
        t[0] = a->rk[4 * (i - 1)];
        t[1] = a->rk[4 * (i - 1) + 1];
        t[2] = a->rk[4 * (i - 1) + 2];
        t[3] = a->rk[4 * (i - 1) + 3];
        if (i % 4 == 0)
        {
            unsigned char t0 = t[0];
            t[0] = sbox[t[1]] ^ rcon;
            t[1] = sbox[t[2]];
            t[2] = sbox[t[3]];
            t[3] = sbox[t0];
            rcon = xtime(rcon);
        }
        for (int j = 0; j < 4; j++)
        {
            a->rk[4 * i + j] = a->rk[4 * (i - 4) + j] ^ t[j];
        }
    }
}

void aes128Encrypt(const Aes128* a, const unsigned char in[16], unsigned char out[16])
{
    unsigned char s[16];
    unsigned char t[16];
    for (int i = 0; i < 16; i++)
    {
        s[i] = in[i] ^ a->rk[i];
    }
    for (int round = 1; round <= 10; round++)
    {
        for (int c = 0; c < 4; c++)
        {
            for (int r = 0; r < 4; r++)
            {
                t[4 * c + r] = sbox[s[4 * ((c + r) & 3) + r]];
            }
        }
        if (round < 10)
        {
            for (int c = 0; c < 4; c++)
            {
                unsigned char a0 = t[4 * c];
                unsigned char a1 = t[4 * c + 1];
                unsigned char a2 = t[4 * c + 2];
                unsigned char a3 = t[4 * c + 3];
                s[4 * c] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
                s[4 * c + 1] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
                s[4 * c + 2] = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
                s[4 * c + 3] = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
            }
        }
        else
        {
            tlsCopy(s, t, 16);
        }
        for (int i = 0; i < 16; i++)
        {
            s[i] ^= a->rk[16 * round + i];
        }
    }
    tlsCopy(out, s, 16);
}

static void gmul(unsigned char* x, const unsigned char* h)
{
    unsigned char z[16];
    unsigned char v[16];
    tlsZero(z, 16);
    tlsCopy(v, h, 16);
    for (int i = 0; i < 128; i++)
    {
        unsigned char bit = (x[i >> 3] >> (7 - (i & 7))) & 1;
        unsigned char mask = (unsigned char)(0 - bit);
        unsigned char lsb = v[15] & 1;
        for (int j = 0; j < 16; j++)
        {
            z[j] ^= v[j] & mask;
        }
        for (int j = 15; j > 0; j--)
        {
            v[j] = (unsigned char)((v[j] >> 1) | (v[j - 1] << 7));
        }
        v[0] = (unsigned char)((v[0] >> 1) ^ (0xe1 & (unsigned char)(0 - lsb)));
    }
    tlsCopy(x, z, 16);
}

static void ghashData(unsigned char* y, const unsigned char* h, const unsigned char* data, unsigned int len)
{
    while (len > 0)
    {
        unsigned int n = len > 16 ? 16 : len;
        for (unsigned int i = 0; i < n; i++)
        {
            y[i] ^= data[i];
        }
        gmul(y, h);
        data += n;
        len -= n;
    }
}

static void putBe32(unsigned char* p, unsigned int v)
{
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v;
}

static void gcmTag(const Gcm* g, const unsigned char iv[12], const unsigned char* aad, unsigned int aadLen, const unsigned char* cipher, unsigned int len, unsigned char tag[16])
{
    unsigned char y[16];
    unsigned char lenBlock[16];
    unsigned char j0[16];
    unsigned char ek[16];
    tlsZero(y, 16);
    ghashData(y, g->h, aad, aadLen);
    ghashData(y, g->h, cipher, len);
    putBe32(lenBlock, aadLen >> 29);
    putBe32(lenBlock + 4, aadLen << 3);
    putBe32(lenBlock + 8, len >> 29);
    putBe32(lenBlock + 12, len << 3);
    ghashData(y, g->h, lenBlock, 16);
    tlsCopy(j0, iv, 12);
    putBe32(j0 + 12, 1);
    aes128Encrypt(&g->aes, j0, ek);
    for (int i = 0; i < 16; i++)
    {
        tag[i] = y[i] ^ ek[i];
    }
}

static void gcmCrypt(const Gcm* g, const unsigned char iv[12], const unsigned char* in, unsigned int len, unsigned char* out)
{
    unsigned char ctr[16];
    unsigned char ks[16];
    unsigned char tmp[16];
    unsigned int counter = 2;
    tlsCopy(ctr, iv, 12);
    while (len > 0)
    {
        unsigned int n = len > 16 ? 16 : len;
        putBe32(ctr + 12, counter++);
        aes128Encrypt(&g->aes, ctr, ks);
        tlsCopy(tmp, in, n);
        for (unsigned int i = 0; i < n; i++)
        {
            out[i] = tmp[i] ^ ks[i];
        }
        in += n;
        out += n;
        len -= n;
    }
}

void gcmInit(Gcm* g, const unsigned char key[16])
{
    unsigned char zero[16];
    aes128SetKey(&g->aes, key);
    tlsZero(zero, 16);
    aes128Encrypt(&g->aes, zero, g->h);
}

void gcmSeal(const Gcm* g, const unsigned char iv[12], const unsigned char* aad, unsigned int aadLen, const unsigned char* in, unsigned int len, unsigned char* out, unsigned char tag[16])
{
    gcmCrypt(g, iv, in, len, out);
    gcmTag(g, iv, aad, aadLen, out, len, tag);
}

int gcmOpen(const Gcm* g, const unsigned char iv[12], const unsigned char* aad, unsigned int aadLen, const unsigned char* in, unsigned int len, const unsigned char tag[16], unsigned char* out)
{
    unsigned char calc[16];
    gcmTag(g, iv, aad, aadLen, in, len, calc);
    if (!tlsConstEq(calc, tag, 16))
    {
        return -1;
    }
    gcmCrypt(g, iv, in, len, out);
    return 0;
}

typedef long long i64;
typedef i64 gf[16];

static const gf gf121665 = {0xDB41, 1};

static void car25519(gf o)
{
    for (int i = 0; i < 16; i++)
    {
        i64 c;
        o[i] += 65536;
        c = o[i] >> 16;
        o[(i + 1) * (i < 15)] += c - 1 + 37 * (c - 1) * (i == 15);
        o[i] -= c * 65536;
    }
}

static void sel25519(gf p, gf q, int b)
{
    i64 c = ~((i64)b - 1);
    for (int i = 0; i < 16; i++)
    {
        i64 t = c & (p[i] ^ q[i]);
        p[i] ^= t;
        q[i] ^= t;
    }
}

static void pack25519(unsigned char* o, const gf n)
{
    gf m;
    gf t;
    for (int i = 0; i < 16; i++)
    {
        t[i] = n[i];
    }
    car25519(t);
    car25519(t);
    car25519(t);
    for (int j = 0; j < 2; j++)
    {
        int b;
        m[0] = t[0] - 0xffed;
        for (int i = 1; i < 15; i++)
        {
            m[i] = t[i] - 0xffff - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xffff;
        }
        m[15] = t[15] - 0x7fff - ((m[14] >> 16) & 1);
        b = (int)((m[15] >> 16) & 1);
        m[14] &= 0xffff;
        sel25519(t, m, 1 - b);
    }
    for (int i = 0; i < 16; i++)
    {
        o[2 * i] = (unsigned char)(t[i] & 0xff);
        o[2 * i + 1] = (unsigned char)(t[i] >> 8);
    }
}

static void unpack25519(gf o, const unsigned char* n)
{
    for (int i = 0; i < 16; i++)
    {
        o[i] = n[2 * i] + ((i64)n[2 * i + 1] << 8);
    }
    o[15] &= 0x7fff;
}

static void fAdd(gf o, const gf a, const gf b)
{
    for (int i = 0; i < 16; i++)
    {
        o[i] = a[i] + b[i];
    }
}

static void fSub(gf o, const gf a, const gf b)
{
    for (int i = 0; i < 16; i++)
    {
        o[i] = a[i] - b[i];
    }
}

static void fMul(gf o, const gf a, const gf b)
{
    i64 t[31];
    for (int i = 0; i < 31; i++)
    {
        t[i] = 0;
    }
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            t[i + j] += a[i] * b[j];
        }
    }
    for (int i = 0; i < 15; i++)
    {
        t[i] += 38 * t[i + 16];
    }
    for (int i = 0; i < 16; i++)
    {
        o[i] = t[i];
    }
    car25519(o);
    car25519(o);
}

static void fSqr(gf o, const gf a)
{
    fMul(o, a, a);
}

static void fInv(gf o, const gf in)
{
    gf c;
    for (int i = 0; i < 16; i++)
    {
        c[i] = in[i];
    }
    for (int a = 253; a >= 0; a--)
    {
        fSqr(c, c);
        if (a != 2 && a != 4)
        {
            fMul(c, c, in);
        }
    }
    for (int i = 0; i < 16; i++)
    {
        o[i] = c[i];
    }
}

void x25519(unsigned char out[32], const unsigned char scalar[32], const unsigned char point[32])
{
    unsigned char z[32];
    i64 x[80];
    gf a;
    gf b;
    gf c;
    gf d;
    gf e;
    gf f;
    for (int i = 0; i < 31; i++)
    {
        z[i] = scalar[i];
    }
    z[31] = (scalar[31] & 127) | 64;
    z[0] &= 248;
    unpack25519(x, point);
    for (int i = 0; i < 16; i++)
    {
        b[i] = x[i];
        d[i] = 0;
        a[i] = 0;
        c[i] = 0;
    }
    a[0] = 1;
    d[0] = 1;
    for (int i = 254; i >= 0; i--)
    {
        int r = (z[i >> 3] >> (i & 7)) & 1;
        sel25519(a, b, r);
        sel25519(c, d, r);
        fAdd(e, a, c);
        fSub(a, a, c);
        fAdd(c, b, d);
        fSub(b, b, d);
        fSqr(d, e);
        fSqr(f, a);
        fMul(a, c, a);
        fMul(c, b, e);
        fAdd(e, a, c);
        fSub(a, a, c);
        fSqr(b, a);
        fSub(c, d, f);
        fMul(a, c, gf121665);
        fAdd(a, a, d);
        fMul(c, c, a);
        fMul(a, d, f);
        fMul(d, b, x);
        fSqr(b, e);
        sel25519(a, b, r);
        sel25519(c, d, r);
    }
    for (int i = 0; i < 16; i++)
    {
        x[i + 16] = a[i];
        x[i + 32] = c[i];
        x[i + 48] = b[i];
        x[i + 64] = d[i];
    }
    fInv(x + 32, x + 32);
    fMul(x + 16, x + 16, x + 32);
    pack25519(out, x + 16);
    tlsZero(z, sizeof(z));
}

void x25519Base(unsigned char out[32], const unsigned char scalar[32])
{
    unsigned char base[32];
    tlsZero(base, 32);
    base[0] = 9;
    x25519(out, scalar, base);
}
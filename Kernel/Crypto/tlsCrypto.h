#ifndef TLS_CRYPTO_H
#define TLS_CRYPTO_H

typedef struct
{
    unsigned int h[8];
    unsigned char buf[64];
    unsigned int bufLen;
    unsigned int lenLo;
    unsigned int lenHi;
} Sha256;

typedef struct
{
    unsigned char rk[176];
} Aes128;

typedef struct
{
    Aes128 aes;
    unsigned char h[16];
} Gcm;

void tlsCopy(void* dst, const void* src, unsigned int len);
void tlsMove(void* dst, const void* src, unsigned int len);
void tlsZero(void* dst, unsigned int len);
int tlsConstEq(const unsigned char* a, const unsigned char* b, unsigned int len);

void sha256Init(Sha256* c);
void sha256Update(Sha256* c, const void* data, unsigned int len);
void sha256Final(Sha256* c, unsigned char out[32]);
void hmacSha256(unsigned char out[32], const unsigned char* key, unsigned int keyLen, const unsigned char* a, unsigned int aLen, const unsigned char* b, unsigned int bLen);

void aes128SetKey(Aes128* a, const unsigned char key[16]);
void aes128Encrypt(const Aes128* a, const unsigned char in[16], unsigned char out[16]);

void gcmInit(Gcm* g, const unsigned char key[16]);
void gcmSeal(const Gcm* g, const unsigned char iv[12], const unsigned char* aad, unsigned int aadLen, const unsigned char* in, unsigned int len, unsigned char* out, unsigned char tag[16]);
int gcmOpen(const Gcm* g, const unsigned char iv[12], const unsigned char* aad, unsigned int aadLen, const unsigned char* in, unsigned int len, const unsigned char tag[16], unsigned char* out);

void x25519(unsigned char out[32], const unsigned char scalar[32], const unsigned char point[32]);
void x25519Base(unsigned char out[32], const unsigned char scalar[32]);

#endif
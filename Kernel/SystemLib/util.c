#include "util.h"

int strLen(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

void strAppend(char* dst, const char* src)
{
    int n = strLen(dst);
    while (*src) dst[n++] = *src++;
    dst[n] = 0;
}

// out needs at least 11 bytes
void utoa10(unsigned int v, char* out)
{
    char tmp[12];
    int n = 0;
    if (v == 0) tmp[n++] = '0';
    while (v)
    {
        tmp[n++] = '0' + v % 10;
        v /= 10;
    }
    for (int i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
    out[n] = 0;
}

int startsWith(const char* s, const char* prefix)
{
    while (*prefix)
    {
        if (*s != *prefix) return 0;
        s++;
        prefix++;
    }
    return 1;
}

int clampInt(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
#include "browser.h"
#include "../Graphics/gfx.h"
#include "../Network/dns.h"
#include "wm.h"
#include "../Crypto/tls.h"

enum
{
    respMax = 65536,
    poolMax = 65536,
    itemsMax = 3072,
    linksMax = 64,
    urlMax = 128,
    charW = 8,
    lineH = 12,
    barH = 22,
    statusH = 14,
    timeoutTicks = 1000,
    dnsRetryTicks = 100,
    dnsMaxTries = 4,
    rulesMax = 48,
    stackMax = 16
};

enum
{
    colBg = 0xFFFFFF,
    colText = 0x202020,
    colLink = 0x0000EE,
    colBar = 0xC8C8C8,
    colRule = 0x808080,
    colScroll = 0x909090
};

typedef enum
{
    brIdle,
    brResolving,
    brConnecting,
    brLoading,
    brDone,
    brError
} BrState;

typedef struct
{
    short x;
    int y;
    unsigned short w;
    unsigned char kind;
    unsigned char bold;
    short link;
    unsigned int color; /* 0 = default, otherwise 0x1000000 | rgb */
    const char* text;
} Item;

/* Result of resolving the (very small) CSS subset for one element. */
typedef struct
{
    unsigned char hasColor;
    unsigned char hasBg;
    unsigned char bold;
    unsigned char hidden;
    unsigned char center;
    unsigned int color;
    unsigned int bg;
} Style;

typedef struct
{
    char tag[12];
    char cls[16];
    char id[16];
    Style st;
} CssRule;

/* Remembers what an element changed so its closing tag can undo it. */
typedef struct
{
    char name[12];
    unsigned int color;
    unsigned char center;
    unsigned char addBold;
    unsigned char addHidden;
} StyleFrame;

static BrState state = brIdle;
static char urlText[urlMax];
static unsigned int urlLen;
static char host[64];
static char path[urlMax];
static unsigned short port;
static char status[64];
static char pageTitle[48];
static unsigned char resp[respMax];
static unsigned int respLen;
static unsigned int bodyStart;
static unsigned int redirects;
static unsigned int lastTick;
static unsigned int lastActivity;
static unsigned int dnsTries;
static unsigned int dnsSent;
static char textPool[poolMax];
static unsigned int poolLen;
static Item items[itemsMax];
static unsigned int itemCount;
static char links[linksMax][urlMax];
static unsigned int linkCount;
static int scrollY;
static int contentH;
static int layoutW;
static int viewW = 600;
static int viewH2;
static int curX;
static int curY;
static int maxW;
static char wordBuf[64];
static unsigned int wordLen;
static unsigned char bold;
static unsigned char heading;
static int curLink;
static int lineUsed;
static int gapped;
static int needSpace;
static unsigned char useTls;
static unsigned char tlsRequestSent;
static TlsConn conn;

/* css / style state */
static CssRule rules[rulesMax];
static unsigned int ruleCount;
static StyleFrame styleStack[stackMax];
static unsigned int styleDepth;
static unsigned int curColor;
static unsigned char center;
static int hidden;
static unsigned int pageBg = colBg;
static unsigned int lineStart;
static unsigned char lineCenter;
static char skipClose[16];

enum
{
    tlsRingSize = 16384,
    tlsChunk = 512
};
static unsigned char tlsIn[tlsRingSize];
static volatile unsigned int tlsHead;
static volatile unsigned int tlsTail;
static volatile unsigned char tlsOverflow;
static unsigned int tlsRxTotal;
static unsigned int tlsShownRx;

static void onData(const unsigned char* data, unsigned int len);

/*
 * TCP receive callback. For https it only pushes the bytes into a ring buffer;
 * the TLS engine is fed from browserPoll so we never send from inside the
 * receive path. The ring has one writer (this callback) and one reader
 * (browserPoll), so bytes that arrive while TLS is busy sending are kept.
 */
static void onTcpData(const unsigned char* data, unsigned int len)
{
    if (!useTls)
    {
        onData(data, len);
        return;
    }
    for (unsigned int i = 0; i < len; i++)
    {
        unsigned int next = (tlsHead + 1) & (tlsRingSize - 1);
        if (next == tlsTail)
        {
            tlsOverflow = 1;
            return;
        }
        tlsIn[tlsHead] = data[i];
        tlsHead = next;
    }
    tlsRxTotal += len;
    lastActivity = lastTick;
}

static void tlsSendOut(void* user, const unsigned char* data, unsigned int len)
{
    tcpSend((void*)data, len);
}

static void tlsGotData(void* user, const unsigned char* data, unsigned int len)
{
    onData(data, len);
}

/* Timer + cycle counter mix. Gets connections working; NOT secure randomness. */
static void fillRandom(unsigned char* out, unsigned int n)
{
    static unsigned int counter;
    unsigned char pool[160];
    unsigned char digest[32];
    unsigned int len = 0;
    unsigned int used = 0;
    Sha256 c;
    for (int i = 0; i < 32; i++)
    {
        unsigned int lo;
        unsigned int hi;
        for (volatile unsigned int spin = (getTicks() + i) & 63; spin > 0; spin--)
        {
        }
        __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
        pool[len++] = lo;
        pool[len++] = lo >> 8;
        pool[len++] = lo >> 16;
        pool[len++] = hi;
    }
    while (used < n)
    {
        unsigned int take = n - used > 32 ? 32 : n - used;
        sha256Init(&c);
        sha256Update(&c, pool, len);
        sha256Update(&c, &counter, sizeof(counter));
        counter++;
        sha256Final(&c, digest);
        tlsCopy(out + used, digest, take);
        used += take;
    }
}

static Color toColor(unsigned int c)
{
    Color k;
    k.r = (c >> 16) & 0xFF;
    k.g = (c >> 8) & 0xFF;
    k.b = c & 0xFF;
    return k;
}

static void putRect(int x, int y, int w, int h, unsigned int color)
{
    gfxFillRect(x, y, w, h, toColor(color));
}

static void putChar(int x, int y, char c, unsigned int color)
{
    char s[2];
    s[0] = c;
    s[1] = '\0';
    gfxDrawString(s, x, y, toColor(color));
}

static unsigned int brLen(const char* s)
{
    unsigned int n = 0;
    while (s[n])
    {
        n++;
    }
    return n;
}

static char brLower(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + 32;
    }
    return c;
}

static int brIsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int brIsAlnum(char c)
{
    return brIsAlpha(c) || (c >= '0' && c <= '9');
}

static int brIsSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static int hexVal(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    c = brLower(c);
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    return -1;
}

static int brEq(const char* a, const char* b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return *a == *b;
}

static int brPrefix(const char* s, const char* p)
{
    while (*p)
    {
        if (brLower(*s) != brLower(*p))
        {
            return 0;
        }
        s++;
        p++;
    }
    return 1;
}

static int brContains(const char* s, const char* sub)
{
    while (*s)
    {
        if (brPrefix(s, sub))
        {
            return 1;
        }
        s++;
    }
    return 0;
}

static const char* findNoCase(const char* p, const char* end, const char* needle)
{
    while (p < end)
    {
        if (brPrefix(p, needle))
        {
            return p;
        }
        p++;
    }
    return end;
}

static void brAppend(char* dst, unsigned int* n, unsigned int max, const char* s)
{
    while (*s && *n < max - 1)
    {
        dst[(*n)++] = *s++;
    }
    dst[*n] = '\0';
}

static void brAppendNum(char* dst, unsigned int* n, unsigned int max, unsigned int v)
{
    char tmp[12];
    unsigned int t = 0;
    char one[2];
    if (v == 0)
    {
        tmp[t++] = '0';
    }
    while (v > 0)
    {
        tmp[t++] = '0' + v % 10;
        v /= 10;
    }
    one[1] = '\0';
    while (t > 0)
    {
        one[0] = tmp[--t];
        brAppend(dst, n, max, one);
    }
}

static void setStatus(const char* msg)
{
    unsigned int n = 0;
    brAppend(status, &n, sizeof(status), msg);
}

static void fail(const char* msg)
{
    setStatus(msg);
    state = brError;
}

static unsigned short defaultPort()
{
    return useTls ? 443 : 80;
}

static void buildOrigin(char* out, unsigned int max)
{
    unsigned int n = 0;
    out[0] = '\0';
    brAppend(out, &n, max, useTls ? "https://" : "http://");
    brAppend(out, &n, max, host);
    if (port != defaultPort())
    {
        brAppend(out, &n, max, ":");
        brAppendNum(out, &n, max, port);
    }
}

static int parseUrl(const char* url)
{
    unsigned int i = 0;
    unsigned int h = 0;
    unsigned int n = 0;
    useTls = 0;
    if (brPrefix(url, "https://"))
    {
        useTls = 1;
        i = 8;
    }
    else if (brPrefix(url, "http://"))
    {
        i = 7;
    }
    while (url[i] && url[i] != '/' && url[i] != ':' && url[i] != '?' && url[i] != '#' && h < sizeof(host) - 1)
    {
        host[h++] = url[i++];
    }
    host[h] = '\0';
    port = defaultPort();
    if (url[i] == ':')
    {
        unsigned int p = 0;
        i++;
        while (url[i] >= '0' && url[i] <= '9')
        {
            p = p * 10 + (url[i] - '0');
            i++;
        }
        port = (unsigned short)p;
    }
    if (url[i] != '/')
    {
        path[n++] = '/';
    }
    while (url[i] && url[i] != '#' && n < sizeof(path) - 1)
    {
        path[n++] = url[i++];
    }
    path[n] = '\0';
    if (h == 0 || port == 0)
    {
        setStatus("bad url");
        return 0;
    }
    return 1;
}

static int resolveLink(const char* href, char* out)
{
    unsigned int n = 0;
    unsigned int i = 0;
    unsigned int last = 0;
    out[0] = '\0';
    if (href[0] == '\0' || href[0] == '#')
    {
        return 0;
    }
    if (brPrefix(href, "http://") || brPrefix(href, "https://"))
    {
        brAppend(out, &n, urlMax, href);
        return 1;
    }
    while (href[i] && href[i] != '/' && href[i] != '?' && href[i] != '#')
    {
        if (href[i] == ':')
        {
            return 0;
        }
        i++;
    }
    if (href[0] == '/' && href[1] == '/')
    {
        brAppend(out, &n, urlMax, useTls ? "https:" : "http:");
        brAppend(out, &n, urlMax, href);
        return 1;
    }
    buildOrigin(out, urlMax);
    n = brLen(out);
    if (href[0] == '/')
    {
        brAppend(out, &n, urlMax, href);
        return 1;
    }
    for (i = 0; path[i] && path[i] != '?'; i++)
    {
        if (path[i] == '/')
        {
            last = i + 1;
        }
    }
    for (i = 0; i < last && n < urlMax - 1; i++)
    {
        out[n++] = path[i];
    }
    out[n] = '\0';
    brAppend(out, &n, urlMax, href);
    return 1;
}

static unsigned int dnsServerIp()
{
    return splitIP((unsigned char*)"10.0.2.3");
}

static unsigned int resolveHost(unsigned int ticks)
{
    unsigned int i = 0;
    unsigned int ip;
    int numeric = 1;
    while (host[i])
    {
        if (!((host[i] >= '0' && host[i] <= '9') || host[i] == '.'))
        {
            numeric = 0;
        }
        i++;
    }
    if (numeric)
    {
        return splitIP((unsigned char*)host);
    }
    ip = dnsResolve(host);
    if (ip)
    {
        return ip;
    }
    if (dnsTries == 0 || ticks - dnsSent >= dnsRetryTicks)
    {
        if (dnsTries >= dnsMaxTries)
        {
            fail("dns lookup timed out");
            return 0;
        }
        dnsSendQuery(myIP, dnsServerIp(), host);
        dnsTries++;
        dnsSent = ticks;
        setStatus("looking up host...");
    }
    return 0;
}

static void onData(const unsigned char* data, unsigned int len)
{
    for (unsigned int i = 0; i < len && respLen < respMax - 1; i++)
    {
        resp[respLen++] = data[i];
    }
    lastActivity = lastTick;
}

/* Builds the GET request and sends it over TLS or plain TCP. */
static void sendRequest()
{
    char req[384];
    unsigned int n = 0;
    brAppend(req, &n, sizeof(req), "GET ");
    brAppend(req, &n, sizeof(req), path);
    brAppend(req, &n, sizeof(req), " HTTP/1.0\r\nHost: ");
    brAppend(req, &n, sizeof(req), host);
    if (port != defaultPort())
    {
        brAppend(req, &n, sizeof(req), ":");
        brAppendNum(req, &n, sizeof(req), port);
    }
    brAppend(req, &n, sizeof(req), "\r\nUser-Agent: obos/0.2\r\nAccept: text/html\r\nConnection: close\r\n\r\n");
    if (useTls)
    {
        tlsWrite(&conn, (const unsigned char*)req, n);
    }
    else
    {
        tcpSend(req, n);
    }
}

/* Centers the words of the current line if it was started inside a centered element. */
static void centerLine()
{
    int shift;
    if (!lineCenter || !lineUsed)
    {
        return;
    }
    shift = (maxW - curX) / 2;
    if (shift <= 0)
    {
        return;
    }
    for (unsigned int i = lineStart; i < itemCount; i++)
    {
        items[i].x += shift;
    }
}

static void breakLine()
{
    centerLine();
    if (lineUsed)
    {
        curY += lineH;
    }
    curX = 0;
    lineUsed = 0;
    needSpace = 0;
}

static void blockBreak()
{
    breakLine();
    if (!gapped && itemCount > 0)
    {
        curY += 6;
        gapped = 1;
    }
}

static void flushWord()
{
    int w;
    int sp;
    if (wordLen == 0)
    {
        return;
    }
    wordBuf[wordLen] = '\0';
    w = wordLen * charW;
    sp = (needSpace && curX > 0) ? charW : 0;
    if (curX + sp + w > maxW && curX > 0)
    {
        breakLine();
        sp = 0;
    }
    curX += sp;
    if (!lineUsed)
    {
        lineStart = itemCount;
        lineCenter = center;
    }
    if (itemCount < itemsMax && poolLen + wordLen + 1 < poolMax)
    {
        Item* it = &items[itemCount++];
        it->x = curX;
        it->y = curY;
        it->w = w;
        it->kind = 0;
        it->bold = bold || heading;
        it->link = curLink;
        it->color = curColor;
        it->text = &textPool[poolLen];
        for (unsigned int i = 0; i <= wordLen; i++)
        {
            textPool[poolLen + i] = wordBuf[i];
        }
        poolLen += wordLen + 1;
    }
    curX += w;
    lineUsed = 1;
    gapped = 0;
    needSpace = 0;
    wordLen = 0;
}

static void addChar(char c)
{
    unsigned int limit = maxW / charW;
    if ((unsigned char)c >= 0x80)
    {
        if ((unsigned char)c < 0xC0)
        {
            return;
        }
        c = '?';
    }
    if (c < 32 || c > 126)
    {
        return;
    }
    if (limit > sizeof(wordBuf) - 1)
    {
        limit = sizeof(wordBuf) - 1;
    }
    if (limit < 1)
    {
        limit = 1;
    }
    if (wordLen >= limit)
    {
        flushWord();
    }
    wordBuf[wordLen++] = c;
}

static void addText(const char* s)
{
    while (*s)
    {
        addChar(*s++);
    }
    flushWord();
}

static void addRule()
{
    if (itemCount < itemsMax)
    {
        Item* it = &items[itemCount++];
        it->x = 0;
        it->y = curY;
        it->w = maxW;
        it->kind = 1;
        it->bold = 0;
        it->link = -1;
        it->color = 0;
        it->text = "";
    }
    curY += lineH;
    gapped = 0;
}

static int attrValue(const char* t, unsigned int len, const char* name, char* out, unsigned int max)
{
    unsigned int nl = brLen(name);
    for (unsigned int i = 1; i + nl < len; i++)
    {
        unsigned int j;
        unsigned int n = 0;
        char q = 0;
        if (!brIsSpace(t[i - 1]) || !brPrefix(t + i, name))
        {
            continue;
        }
        j = i + nl;
        while (j < len && brIsSpace(t[j]))
        {
            j++;
        }
        if (j >= len || t[j] != '=')
        {
            continue;
        }
        j++;
        while (j < len && brIsSpace(t[j]))
        {
            j++;
        }
        if (j < len && (t[j] == '"' || t[j] == '\''))
        {
            q = t[j];
            j++;
        }
        while (j < len && n < max - 1)
        {
            if (q ? t[j] == q : brIsSpace(t[j]))
            {
                break;
            }
            out[n++] = t[j++];
        }
        out[n] = '\0';
        return n > 0;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Tag lists                                                           */
/* ------------------------------------------------------------------ */

static const char* const voidTags[] = {"br", "hr", "img", "input", "meta", "link", "area", "base", "col", "embed", "param", "source", "track", "wbr", 0};

/* content of these tags is never shown */
static const char* const skipTags[] = {"svg", "template", "datalist", 0};

/* tags that start a new paragraph-like block (blank gap before) */
static const char* const blockTags[] = {"p", "ul", "ol", "table", "blockquote", "pre", "form", "dl", "fieldset", "figure", "address", "details", "textarea", "menu", "dir", 0};

/* tags that only start a new line */
static const char* const lineTags[] = {"div", "tr", "dt", "dd", "section", "article", "header", "footer", "nav", "main", "aside", "figcaption", "summary", "legend", "caption", "thead", "tbody", "tfoot", "center", "option", "optgroup", "hgroup", 0};

/* tags that are often left unclosed; a new one ends the previous one's style */
static const char* const autoCloseTags[] = {"p", "li", "dt", "dd", "th", "td", "tr", "option", 0};

/* default bold elements */
static const char* const boldTags[] = {"th", "dt", "caption", "legend", "summary", 0};

typedef struct
{
    const char* tag;
    unsigned int rgb;
} TagColor;

/* tiny built-in "user agent stylesheet" (we only have one font, so colour stands in for italics etc.) */
static const TagColor tagColors[] = {
    {"del", 0x808080}, {"s", 0x808080}, {"strike", 0x808080}, {"small", 0x606060},
    {"code", 0x006400}, {"kbd", 0x006400}, {"samp", 0x006400}, {"tt", 0x006400},
    {"em", 0x603060}, {"i", 0x603060}, {"cite", 0x603060}, {"dfn", 0x603060}, {"var", 0x603060},
    {"mark", 0xB05000}, {"blockquote", 0x505050},
    {0, 0}};

typedef struct
{
    const char* name;
    unsigned int rgb;
} NamedColor;

static const NamedColor namedColors[] = {
    {"black", 0x000000}, {"white", 0xFFFFFF}, {"red", 0xFF0000}, {"green", 0x008000},
    {"blue", 0x0000FF}, {"yellow", 0xFFFF00}, {"gray", 0x808080}, {"grey", 0x808080},
    {"silver", 0xC0C0C0}, {"orange", 0xFFA500}, {"purple", 0x800080}, {"navy", 0x000080},
    {"maroon", 0x800000}, {"teal", 0x008080}, {"lime", 0x00FF00}, {"olive", 0x808000},
    {"aqua", 0x00FFFF}, {"cyan", 0x00FFFF}, {"fuchsia", 0xFF00FF}, {"magenta", 0xFF00FF},
    {"brown", 0xA52A2A}, {"pink", 0xFFC0CB}, {"gold", 0xFFD700}, {"crimson", 0xDC143C},
    {0, 0}};

static int inList(const char* name, const char* const* list)
{
    for (unsigned int i = 0; list[i]; i++)
    {
        if (brEq(name, list[i]))
        {
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Simple CSS                                                          */
/* Supported properties: color, background(-color) on body/html,      */
/* font-weight, display:none, visibility:hidden, text-align:center.   */
/* Selectors: tag, .class, #id and combinations like p.note.          */
/* ------------------------------------------------------------------ */

static void clearStyle(Style* s)
{
    s->hasColor = 0;
    s->hasBg = 0;
    s->bold = 0;
    s->hidden = 0;
    s->center = 0;
    s->color = 0;
    s->bg = 0;
}

static void mergeStyle(Style* d, const Style* s)
{
    if (s->hasColor)
    {
        d->hasColor = 1;
        d->color = s->color;
    }
    if (s->hasBg)
    {
        d->hasBg = 1;
        d->bg = s->bg;
    }
    if (s->bold)
    {
        d->bold = 1;
    }
    if (s->hidden)
    {
        d->hidden = 1;
    }
    if (s->center)
    {
        d->center = 1;
    }
}

static int parseColor(const char* v, unsigned int* out)
{
    while (brIsSpace(*v))
    {
        v++;
    }
    if (*v == '#')
    {
        unsigned int d = 0;
        unsigned int val = 0;
        v++;
        while (d < 6 && hexVal(v[d]) >= 0)
        {
            val = val * 16 + hexVal(v[d]);
            d++;
        }
        if (d == 6)
        {
            *out = val;
            return 1;
        }
        if (d == 3)
        {
            *out = ((((val >> 8) & 15) * 17) << 16) | ((((val >> 4) & 15) * 17) << 8) | ((val & 15) * 17);
            return 1;
        }
        return 0;
    }
    if (brPrefix(v, "rgb"))
    {
        unsigned int comp[3];
        while (*v && *v != '(')
        {
            v++;
        }
        if (*v != '(')
        {
            return 0;
        }
        v++;
        for (int k = 0; k < 3; k++)
        {
            unsigned int num = 0;
            while (*v == ' ' || *v == ',' || *v == '\t')
            {
                v++;
            }
            if (*v < '0' || *v > '9')
            {
                return 0;
            }
            while (*v >= '0' && *v <= '9')
            {
                if (num < 1000)
                {
                    num = num * 10 + (*v - '0');
                }
                v++;
            }
            if (*v == '%')
            {
                num = num * 255 / 100;
                v++;
            }
            if (num > 255)
            {
                num = 255;
            }
            comp[k] = num;
        }
        *out = (comp[0] << 16) | (comp[1] << 8) | comp[2];
        return 1;
    }
    if (brIsAlpha(*v))
    {
        char tok[12];
        unsigned int n = 0;
        while (brIsAlpha(*v) && n < sizeof(tok) - 1)
        {
            tok[n++] = brLower(*v++);
        }
        tok[n] = '\0';
        for (unsigned int i = 0; namedColors[i].name; i++)
        {
            if (brEq(tok, namedColors[i].name))
            {
                *out = namedColors[i].rgb;
                return 1;
            }
        }
    }
    return 0;
}

static void applyDecl(const char* prop, const char* val, Style* st)
{
    unsigned int c;
    if (brEq(prop, "color"))
    {
        if (parseColor(val, &c))
        {
            st->hasColor = 1;
            st->color = c;
        }
    }
    else if (brEq(prop, "background-color") || brEq(prop, "background"))
    {
        if (parseColor(val, &c))
        {
            st->hasBg = 1;
            st->bg = c;
        }
    }
    else if (brEq(prop, "font-weight"))
    {
        if (brPrefix(val, "bold"))
        {
            st->bold = 1;
        }
        else if (val[0] >= '0' && val[0] <= '9')
        {
            unsigned int w = 0;
            for (unsigned int i = 0; val[i] >= '0' && val[i] <= '9' && i < 5; i++)
            {
                w = w * 10 + (val[i] - '0');
            }
            if (w >= 600)
            {
                st->bold = 1;
            }
        }
    }
    else if (brEq(prop, "display"))
    {
        if (brPrefix(val, "none"))
        {
            st->hidden = 1;
        }
    }
    else if (brEq(prop, "visibility"))
    {
        if (brPrefix(val, "hidden"))
        {
            st->hidden = 1;
        }
    }
    else if (brEq(prop, "text-align"))
    {
        if (brPrefix(val, "center"))
        {
            st->center = 1;
        }
    }
}

/* Parses "prop: value; prop: value" between s and e. */
static void parseDecls(const char* s, const char* e, Style* st)
{
    while (s < e)
    {
        char prop[24];
        char val[40];
        unsigned int pn = 0;
        unsigned int vn = 0;
        while (s < e && (brIsSpace(*s) || *s == ';'))
        {
            s++;
        }
        while (s < e && *s != ':' && *s != ';')
        {
            if (pn < sizeof(prop) - 1)
            {
                prop[pn++] = brLower(*s);
            }
            s++;
        }
        while (pn > 0 && brIsSpace(prop[pn - 1]))
        {
            pn--;
        }
        prop[pn] = '\0';
        if (s >= e || *s == ';')
        {
            s++;
            continue;
        }
        s++;
        while (s < e && brIsSpace(*s))
        {
            s++;
        }
        while (s < e && *s != ';')
        {
            if (vn < sizeof(val) - 1)
            {
                val[vn++] = *s;
            }
            s++;
        }
        val[vn] = '\0';
        s++;
        applyDecl(prop, val, st);
    }
}

static void addCssRule(const char* s, const char* e, const Style* st)
{
    CssRule* r;
    unsigned int n = 0;
    while (s < e && brIsSpace(*s))
    {
        s++;
    }
    while (e > s && brIsSpace(e[-1]))
    {
        e--;
    }
    if (s >= e || ruleCount >= rulesMax || *s == '*')
    {
        return;
    }
    r = &rules[ruleCount];
    r->tag[0] = '\0';
    r->cls[0] = '\0';
    r->id[0] = '\0';
    while (s < e && brIsAlnum(*s))
    {
        if (n < sizeof(r->tag) - 1)
        {
            r->tag[n++] = brLower(*s);
            r->tag[n] = '\0';
        }
        s++;
    }
    while (s < e)
    {
        char kind = *s++;
        char* dst;
        n = 0;
        if (kind == '.')
        {
            dst = r->cls;
        }
        else if (kind == '#')
        {
            dst = r->id;
        }
        else
        {
            return; /* descendant, pseudo, attribute selectors etc. are not supported */
        }
        while (s < e && (brIsAlnum(*s) || *s == '-' || *s == '_'))
        {
            if (n < 15)
            {
                dst[n++] = *s;
                dst[n] = '\0';
            }
            s++;
        }
    }
    if (!r->tag[0] && !r->cls[0] && !r->id[0])
    {
        return;
    }
    clearStyle(&r->st);
    mergeStyle(&r->st, st);
    ruleCount++;
}

/* Parses the text of a <style> block into the rule table. */
static void parseCss(const char* s, const char* e)
{
    while (s < e)
    {
        const char* selStart;
        const char* selEnd;
        const char* declStart;
        const char* a;
        Style st;
        while (s < e && brIsSpace(*s))
        {
            s++;
        }
        if (s + 1 < e && s[0] == '/' && s[1] == '*')
        {
            s += 2;
            while (s + 1 < e && !(s[0] == '*' && s[1] == '/'))
            {
                s++;
            }
            s += 2;
            continue;
        }
        if (s >= e)
        {
            break;
        }
        selStart = s;
        while (s < e && *s != '{' && *s != ';')
        {
            s++;
        }
        if (s >= e)
        {
            break;
        }
        if (*s == ';')
        {
            s++;
            continue;
        }
        selEnd = s;
        s++;
        if (*selStart == '@')
        {
            int depth = 1;
            while (s < e && depth > 0)
            {
                if (*s == '{')
                {
                    depth++;
                }
                else if (*s == '}')
                {
                    depth--;
                }
                s++;
            }
            continue;
        }
        declStart = s;
        while (s < e && *s != '}')
        {
            s++;
        }
        clearStyle(&st);
        parseDecls(declStart, s, &st);
        if (s < e)
        {
            s++;
        }
        a = selStart;
        for (const char* c = selStart; c <= selEnd; c++)
        {
            if (c == selEnd || *c == ',')
            {
                addCssRule(a, c, &st);
                a = c + 1;
            }
        }
    }
}

static int hasClass(const char* list, const char* cls)
{
    unsigned int cl = brLen(cls);
    while (*list)
    {
        unsigned int n = 0;
        while (*list && brIsSpace(*list))
        {
            list++;
        }
        while (list[n] && !brIsSpace(list[n]))
        {
            n++;
        }
        if (n == cl && n > 0)
        {
            unsigned int k = 0;
            while (k < n && list[k] == cls[k])
            {
                k++;
            }
            if (k == n)
            {
                return 1;
            }
        }
        list += n;
    }
    return 0;
}

static int ruleMatches(const CssRule* r, const char* name, const char* cls, const char* id)
{
    if (r->tag[0] && !brEq(r->tag, name))
    {
        return 0;
    }
    if (r->cls[0] && !hasClass(cls, r->cls))
    {
        return 0;
    }
    if (r->id[0] && !brEq(r->id, id))
    {
        return 0;
    }
    return 1;
}

/* Works out the style of one element from defaults, attributes, <style> rules and style="". */
static void computeStyle(const char* name, const char* t, unsigned int len, Style* st)
{
    char cls[64];
    char id[32];
    char val[160];
    unsigned int c;
    clearStyle(st);
    cls[0] = '\0';
    id[0] = '\0';
    attrValue(t, len, "class", cls, sizeof(cls));
    attrValue(t, len, "id", id, sizeof(id));

    if (inList(name, boldTags))
    {
        st->bold = 1;
    }
    for (unsigned int i = 0; tagColors[i].tag; i++)
    {
        if (brEq(name, tagColors[i].tag))
        {
            st->hasColor = 1;
            st->color = tagColors[i].rgb;
        }
    }
    if (brEq(name, "center"))
    {
        st->center = 1;
    }
    if (attrValue(t, len, "align", val, sizeof(val)) && brPrefix(val, "center"))
    {
        st->center = 1;
    }
    if (brEq(name, "font") && attrValue(t, len, "color", val, sizeof(val)) && parseColor(val, &c))
    {
        st->hasColor = 1;
        st->color = c;
    }
    if (brEq(name, "body"))
    {
        if (attrValue(t, len, "bgcolor", val, sizeof(val)) && parseColor(val, &c))
        {
            st->hasBg = 1;
            st->bg = c;
        }
        if (attrValue(t, len, "text", val, sizeof(val)) && parseColor(val, &c))
        {
            st->hasColor = 1;
            st->color = c;
        }
    }
    for (int pass = 0; pass < 3; pass++)
    {
        for (unsigned int i = 0; i < ruleCount; i++)
        {
            const CssRule* r = &rules[i];
            int level = r->id[0] ? 2 : (r->cls[0] ? 1 : 0);
            if (level == pass && ruleMatches(r, name, cls, id))
            {
                mergeStyle(st, &r->st);
            }
        }
    }
    if (attrValue(t, len, "style", val, sizeof(val)))
    {
        parseDecls(val, val + brLen(val), st);
    }
}

static void styleOpen(const char* name, const Style* st)
{
    StyleFrame* f;
    unsigned int i = 0;
    if (!st->hasColor && !st->bold && !st->hidden && !st->center)
    {
        return;
    }
    if (styleDepth >= stackMax)
    {
        return;
    }
    f = &styleStack[styleDepth++];
    while (name[i] && i < sizeof(f->name) - 1)
    {
        f->name[i] = name[i];
        i++;
    }
    f->name[i] = '\0';
    f->color = curColor;
    f->center = center;
    f->addBold = st->bold;
    f->addHidden = st->hidden;
    if (st->hasColor)
    {
        curColor = st->color | 0x1000000;
    }
    if (st->center)
    {
        center = 1;
    }
    if (st->bold)
    {
        bold++;
    }
    if (st->hidden)
    {
        hidden++;
    }
}

static void styleClose(const char* name)
{
    int k = (int)styleDepth - 1;
    while (k >= 0 && !brEq(styleStack[k].name, name))
    {
        k--;
    }
    if (k < 0)
    {
        return;
    }
    while ((int)styleDepth > k)
    {
        StyleFrame* f = &styleStack[--styleDepth];
        curColor = f->color;
        center = f->center;
        if (f->addBold && bold)
        {
            bold--;
        }
        if (f->addHidden && hidden)
        {
            hidden--;
        }
    }
}

static void openStyle(const char* name, const char* t, unsigned int len)
{
    Style st;
    char tmp[4];
    computeStyle(name, t, len, &st);
    if ((brEq(name, "body") || brEq(name, "html")) && st.hasBg)
    {
        pageBg = st.bg;
    }
    if (brEq(name, "a") && !st.hasColor && attrValue(t, len, "href", tmp, sizeof(tmp)))
    {
        st.hasColor = 1;
        st.color = colLink;
    }
    if (inList(name, autoCloseTags) && styleDepth > 0 && brEq(styleStack[styleDepth - 1].name, name))
    {
        styleClose(name);
    }
    styleOpen(name, &st);
}

/* Draws a form control as plain text, e.g. [____], [ok], [ ], ( ) */
static void addInput(const char* t, unsigned int len)
{
    char type[12];
    char val[24];
    char buf[40];
    unsigned int n = 0;
    int isButton;
    type[0] = '\0';
    val[0] = '\0';
    attrValue(t, len, "type", type, sizeof(type));
    attrValue(t, len, "value", val, sizeof(val));
    if (brPrefix(type, "hidden"))
    {
        return;
    }
    needSpace = 1;
    if (brPrefix(type, "checkbox"))
    {
        addText("[ ]");
    }
    else if (brPrefix(type, "radio"))
    {
        addText("( )");
    }
    else if (brPrefix(type, "image"))
    {
        addText("[img]");
    }
    else
    {
        isButton = brPrefix(type, "submit") || brPrefix(type, "button") || brPrefix(type, "reset");
        brAppend(buf, &n, sizeof(buf), "[");
        brAppend(buf, &n, sizeof(buf), val[0] ? val : (isButton ? type : "____"));
        brAppend(buf, &n, sizeof(buf), "]");
        addText(buf);
    }
}

static int handleTag(const char* t, unsigned int len)
{
    char name[12];
    unsigned int n = 0;
    unsigned int i = 0;
    int closing = 0;
    if (len > 0 && t[0] == '/')
    {
        closing = 1;
        i = 1;
    }
    while (i < len && n < sizeof(name) - 1 && brIsAlnum(t[i]))
    {
        name[n++] = brLower(t[i]);
        i++;
    }
    name[n] = '\0';
    flushWord();

    if (brEq(name, "script"))
    {
        return closing ? 0 : 1;
    }
    if (brEq(name, "style"))
    {
        return closing ? 0 : 2;
    }
    if (brEq(name, "title"))
    {
        return closing ? 4 : 3;
    }
    if (!closing && inList(name, skipTags))
    {
        unsigned int k = 0;
        skipClose[0] = '<';
        skipClose[1] = '/';
        while (name[k])
        {
            skipClose[2 + k] = name[k];
            k++;
        }
        skipClose[2 + k] = '\0';
        return 5;
    }

    if (closing)
    {
        styleClose(name);
    }
    else if (!inList(name, voidTags))
    {
        openStyle(name, t, len);
    }
    if (hidden)
    {
        return 0;
    }

    if (name[0] == 'h' && name[1] >= '1' && name[1] <= '6' && name[2] == '\0')
    {
        blockBreak();
        heading = !closing;
    }
    else if (brEq(name, "br"))
    {
        if (lineUsed)
        {
            breakLine();
        }
        else
        {
            curY += lineH;
        }
    }
    else if (inList(name, blockTags))
    {
        blockBreak();
    }
    else if (inList(name, lineTags))
    {
        breakLine();
    }
    else if (brEq(name, "td") || brEq(name, "th"))
    {
        needSpace = 1;
    }
    else if (brEq(name, "li"))
    {
        breakLine();
        if (!closing)
        {
            addText("*");
            needSpace = 1;
        }
    }
    else if (brEq(name, "b") || brEq(name, "strong"))
    {
        if (!closing)
        {
            bold++;
        }
        else if (bold)
        {
            bold--;
        }
    }
    else if (brEq(name, "a"))
    {
        curLink = -1;
        if (!closing && linkCount < linksMax && attrValue(t, len, "href", links[linkCount], urlMax))
        {
            curLink = linkCount++;
        }
    }
    else if (brEq(name, "hr"))
    {
        blockBreak();
        addRule();
        blockBreak();
    }
    else if (brEq(name, "img"))
    {
        needSpace = 1;
        addText("[img]");
    }
    else if (brEq(name, "input"))
    {
        addInput(t, len);
    }
    else if (brEq(name, "button"))
    {
        if (!closing)
        {
            needSpace = 1;
        }
        addText(closing ? "]" : "[");
    }
    else if (brEq(name, "q"))
    {
        addText("\"");
    }
    return 0;
}

/* Maps a unicode code point to the closest printable ASCII char. */
static char mapCode(unsigned int v)
{
    if (v >= 32 && v < 127)
    {
        return (char)v;
    }
    switch (v)
    {
    case 0xA0:
        return ' ';
    case 0xAB:
    case 0xBB:
    case 0x201C:
    case 0x201D:
        return '"';
    case 0x2018:
    case 0x2019:
        return '\'';
    case 0x2013:
    case 0x2014:
        return '-';
    case 0x2022:
    case 0xB7:
        return '*';
    case 0x2026:
        return '.';
    case 0xD7:
        return 'x';
    }
    return '?';
}

static unsigned int entity(const char* p, const char* end, char* out)
{
    static const char* const names[] = {"amp;", "lt;", "gt;", "quot;", "nbsp;", "apos;", "laquo;", "raquo;", "ldquo;", "rdquo;", "lsquo;", "rsquo;", "ndash;", "mdash;", "bull;", "middot;", "hellip;", "times;"};
    static const unsigned short vals[] = {'&', '<', '>', '"', 0xA0, '\'', 0xAB, 0xBB, 0x201C, 0x201D, 0x2018, 0x2019, 0x2013, 0x2014, 0x2022, 0xB7, 0x2026, 0xD7};
    if (p + 1 >= end)
    {
        return 0;
    }
    if (p[1] == '#')
    {
        unsigned int v = 0;
        unsigned int i = 2;
        unsigned int base = 10;
        unsigned int digits = 0;
        if (p[i] == 'x' || p[i] == 'X')
        {
            base = 16;
            i++;
        }
        while (p + i < end && i < 10)
        {
            int d = hexVal(p[i]);
            if (d < 0 || (unsigned int)d >= base)
            {
                break;
            }
            v = v * base + d;
            digits++;
            i++;
        }
        if (digits == 0)
        {
            return 0;
        }
        if (p + i < end && p[i] == ';')
        {
            i++;
        }
        *out = mapCode(v);
        return i;
    }
    for (unsigned int k = 0; k < sizeof(vals) / sizeof(vals[0]); k++)
    {
        if (brPrefix(p + 1, names[k]))
        {
            *out = mapCode(vals[k]);
            return brLen(names[k]) + 1;
        }
    }
    return 0;
}

static void layoutPage(int width)
{
    const char* p = (const char*)resp + bodyStart;
    const char* end = (const char*)resp + respLen;
    int inTitle = 0;
    unsigned int titleLen = 0;
    layoutW = width;
    maxW = width - 8;
    if (maxW < 64)
    {
        maxW = 64;
    }
    itemCount = 0;
    poolLen = 0;
    linkCount = 0;
    curX = 0;
    curY = 0;
    wordLen = 0;
    bold = 0;
    heading = 0;
    curLink = -1;
    lineUsed = 0;
    gapped = 1;
    needSpace = 0;
    ruleCount = 0;
    styleDepth = 0;
    curColor = 0;
    center = 0;
    hidden = 0;
    lineStart = 0;
    lineCenter = 0;
    pageBg = colBg;
    pageTitle[0] = '\0';
    while (p < end)
    {
        char c = *p;
        if (c == '<' && (brIsAlpha(p[1]) || p[1] == '/' || p[1] == '!'))
        {
            const char* close;
            int action;
            if (p[1] == '!' && p[2] == '-' && p[3] == '-')
            {
                p = findNoCase(p + 4, end, "-->");
                if (p < end)
                {
                    p += 3;
                }
                continue;
            }
            close = p + 1;
            while (close < end && *close != '>')
            {
                close++;
            }
            action = handleTag(p + 1, close - p - 1);
            p = close < end ? close + 1 : end;
            if (action == 1)
            {
                p = findNoCase(p, end, "</script");
            }
            else if (action == 2)
            {
                const char* css = p;
                p = findNoCase(p, end, "</style");
                parseCss(css, p);
            }
            else if (action == 3)
            {
                inTitle = 1;
            }
            else if (action == 4)
            {
                inTitle = 0;
            }
            else if (action == 5)
            {
                p = findNoCase(p, end, skipClose);
            }
            continue;
        }
        if (inTitle)
        {
            if (c >= 32 && c < 127 && titleLen < sizeof(pageTitle) - 1)
            {
                pageTitle[titleLen++] = c;
                pageTitle[titleLen] = '\0';
            }
            p++;
            continue;
        }
        if (hidden)
        {
            p++;
            continue;
        }
        if (c == '&')
        {
            char ch;
            unsigned int used = entity(p, end, &ch);
            if (used)
            {
                if (ch == ' ')
                {
                    flushWord();
                    needSpace = 1;
                }
                else
                {
                    addChar(ch);
                }
                p += used;
                continue;
            }
        }
        if (brIsSpace(c))
        {
            flushWord();
            needSpace = 1;
        }
        else
        {
            addChar(c);
        }
        p++;
    }
    flushWord();
    centerLine();
    contentH = curY + (lineUsed ? lineH : 0) + lineH;
}

static unsigned int findHeaderEnd()
{
    for (unsigned int i = 0; i + 3 < respLen; i++)
    {
        if (resp[i] == '\r' && resp[i + 1] == '\n' && resp[i + 2] == '\r' && resp[i + 3] == '\n')
        {
            return i + 4;
        }
    }
    return 0;
}

static int headerValue(const char* name, unsigned int hdrEnd, char* out, unsigned int max)
{
    unsigned int i = 0;
    while (i < hdrEnd)
    {
        if (brPrefix((const char*)resp + i, name))
        {
            unsigned int n = 0;
            i += brLen(name);
            while (resp[i] == ' ')
            {
                i++;
            }
            while (resp[i] && resp[i] != '\r' && resp[i] != '\n' && n < max - 1)
            {
                out[n++] = resp[i++];
            }
            out[n] = '\0';
            return 1;
        }
        while (i < hdrEnd && resp[i] != '\n')
        {
            i++;
        }
        i++;
    }
    return 0;
}

static unsigned int dechunk(unsigned int start)
{
    unsigned int src = start;
    unsigned int dst = start;
    while (src < respLen)
    {
        unsigned int size = 0;
        while (src < respLen && hexVal(resp[src]) >= 0)
        {
            size = size * 16 + hexVal(resp[src]);
            src++;
        }
        while (src < respLen && resp[src] != '\n')
        {
            src++;
        }
        src++;
        if (size == 0)
        {
            break;
        }
        for (unsigned int k = 0; k < size && src < respLen; k++)
        {
            resp[dst++] = resp[src++];
        }
        src += 2;
    }
    resp[dst] = '\0';
    return dst;
}

static void startLoad(const char* url)
{
    unsigned int n;
    if (!parseUrl(url))
    {
        state = brError;
        return;
    }
    buildOrigin(urlText, urlMax);
    n = brLen(urlText);
    brAppend(urlText, &n, urlMax, path);
    urlLen = n;
    respLen = 0;
    resp[0] = '\0';
    tlsHead = 0;
    tlsTail = 0;
    tlsOverflow = 0;
    tlsRxTotal = 0;
    tlsShownRx = 0;
    tlsRequestSent = 0;
    lastActivity = lastTick;
    dnsTries = 0;
    setStatus("resolving...");
    state = brResolving;
}

static void finish()
{
    unsigned int code = 0;
    unsigned int i = 0;
    unsigned int hdrEnd;
    char value[urlMax];
    char msg[40];
    unsigned int n = 0;
    resp[respLen] = '\0';
    if (!brPrefix((const char*)resp, "HTTP/"))
    {
        fail("bad response");
        return;
    }
    while (resp[i] && resp[i] != ' ')
    {
        i++;
    }
    while (resp[i] == ' ')
    {
        i++;
    }
    while (resp[i] >= '0' && resp[i] <= '9')
    {
        code = code * 10 + (resp[i] - '0');
        i++;
    }
    hdrEnd = findHeaderEnd();
    if (!hdrEnd)
    {
        fail("truncated response");
        return;
    }
    if (code == 301 || code == 302 || code == 303 || code == 307 || code == 308)
    {
        char target[urlMax];
        if (redirects < 5 && headerValue("location:", hdrEnd, value, sizeof(value)) && resolveLink(value, target))
        {
            redirects++;
            startLoad(target);
            return;
        }
        fail("bad redirect");
        return;
    }
    if (headerValue("content-type:", hdrEnd, value, sizeof(value)) && !brContains(value, "text/"))
    {
        fail("unsupported content type");
        return;
    }
    bodyStart = hdrEnd;
    if (headerValue("transfer-encoding:", hdrEnd, value, sizeof(value)) && brContains(value, "chunked"))
    {
        respLen = dechunk(hdrEnd);
    }
    scrollY = 0;
    layoutPage(viewW);
    if (useTls)
    {
        brAppend(msg, &n, sizeof(msg), "https (unverified) ");
    }
    else
    {
        brAppend(msg, &n, sizeof(msg), "http ");
    }
    brAppendNum(msg, &n, sizeof(msg), code);
    setStatus(msg);
    state = brDone;
}

void browserInit()
{
    state = brIdle;
    itemCount = 0;
    contentH = 0;
    scrollY = 0;
    urlLen = 0;
    urlText[0] = '\0';
    pageTitle[0] = '\0';
    useTls = 0;
    pageBg = colBg;
    setStatus("type a url and press enter");
    brAppend(urlText, &urlLen, urlMax, "http://");
}

void browserOpen(const char* url)
{
    if (state == brResolving || state == brConnecting || state == brLoading)
    {
        return;
    }
    redirects = 0;
    startLoad(url);
}

static void tlsFail(const char* what)
{
    char msg[64];
    unsigned int n = 0;
    brAppend(msg, &n, sizeof(msg), what);
    brAppend(msg, &n, sizeof(msg), " rx=");
    brAppendNum(msg, &n, sizeof(msg), tlsRxTotal);
    brAppend(msg, &n, sizeof(msg), " st=");
    brAppendNum(msg, &n, sizeof(msg), (unsigned int)tlsGetState(&conn));
    brAppend(msg, &n, sizeof(msg), " al=");
    brAppendNum(msg, &n, sizeof(msg), (unsigned int)conn.alertCode);
    fail(msg);
}

static int pumpTls()
{
    unsigned char chunk[tlsChunk];
    while (tlsTail != tlsHead)
    {
        unsigned int n = 0;
        while (n < sizeof(chunk) && tlsTail != tlsHead)
        {
            chunk[n++] = tlsIn[tlsTail];
            tlsTail = (tlsTail + 1) & (tlsRingSize - 1);
        }
        tlsFeed(&conn, chunk, n);
        if (tlsGetState(&conn) == tlsFailed)
        {
            break;
        }
    }
    if (tlsOverflow)
    {
        tcpAbort();
        fail("tls receive buffer overflow");
        return 0;
    }
    if (tlsGetState(&conn) == tlsFailed)
    {
        tcpAbort();
        fail(tlsErrorText(&conn));
        return 0;
    }
    if (tlsGetState(&conn) == tlsReady && !tlsRequestSent)
    {
        tlsRequestSent = 1;
        sendRequest();
        lastActivity = lastTick;
        setStatus("loading...");
    }
    else if (!tlsRequestSent && tlsRxTotal != tlsShownRx)
    {
        char msg[48];
        unsigned int n = 0;
        tlsShownRx = tlsRxTotal;
        brAppend(msg, &n, sizeof(msg), "tls handshake: ");
        brAppendNum(msg, &n, sizeof(msg), tlsRxTotal);
        brAppend(msg, &n, sizeof(msg), " bytes in");
        setStatus(msg);
    }
    return 1;
}

void browserPoll(unsigned int ticks)
{
    TcpState ts;
    int tick = ticks != lastTick;
    lastTick = ticks;
    if (state == brResolving)
    {
        unsigned int ip = resolveHost(ticks);
        if (ip == 0)
        {
            return;
        }
        if (tcpConnect(ip, port, onTcpData, 0) != SUCCESS)
        {
            fail("tcp busy");
            return;
        }
        lastActivity = ticks;
        setStatus("connecting...");
        state = brConnecting;
        return;
    }
    if (state != brConnecting && state != brLoading)
    {
        return;
    }
    if (tick)
    {
        tcpTick();
    }
    ts = tcpGetState();
    if (state == brConnecting)
    {
        if (ts == TCP_ESTABLISHED)
        {
            if (useTls)
            {
                unsigned char rnd[64];
                fillRandom(rnd, 64);
                tlsInit(&conn, host, rnd, tlsSendOut, tlsGotData, 0);
                tlsStart(&conn);
                setStatus("tls handshake...");
            }
            else
            {
                sendRequest();
                setStatus("loading...");
            }
            lastActivity = ticks;
            state = brLoading;
        }
        else if (ts == TCP_CLOSED)
        {
            fail("connection failed");
        }
        return;
    }
    if (useTls && !pumpTls())
    {
        return;
    }
    if (ts == TCP_CLOSED)
    {
        if (useTls && respLen == 0)
        {
            tlsFail("tls closed early");
        }
        else
        {
            finish();
        }
    }
    else if (ticks - lastActivity > timeoutTicks)
    {
        tcpAbort();
        if (respLen > 0)
        {
            finish();
        }
        else if (useTls)
        {
            tlsFail("tls timed out");
        }
        else
        {
            fail("timed out");
        }
    }
}

static void drawStr(int x, int y, const char* s, unsigned int color)
{
    for (unsigned int i = 0; s[i]; i++)
    {
        putChar(x + i * charW, y, s[i], color);
    }
}

static void drawWord(int x, int y, const Item* it)
{
    unsigned int color = colText;
    if (it->color)
    {
        color = it->color & 0xFFFFFF;
    }
    else if (it->link >= 0)
    {
        color = colLink;
    }
    for (unsigned int i = 0; it->text[i]; i++)
    {
        putChar(x + i * charW, y + 1, it->text[i], color);
        if (it->bold)
        {
            putChar(x + i * charW + 1, y + 1, it->text[i], color);
        }
    }
    if (it->link >= 0)
    {
        putRect(x, y + 10, it->w, 1, color);
    }
}
static int ready = 0;

void browserDraw(int x, int y, int w, int h)
{
    if (!ready)
    {
        browserInit();
        ready = 1;
    }
    int top = y + barH;
    int bottom = y + h - statusH;
    int viewH = bottom - top;
    int maxScroll;
    unsigned int maxChars = (w - 16) / charW;
    unsigned int start = urlLen > maxChars ? urlLen - maxChars : 0;
    viewW = w;
    viewH2 = h;
    if (state == brDone && layoutW != w)
    {
        layoutPage(w);
    }
    maxScroll = contentH - viewH;
    if (maxScroll < 0)
    {
        maxScroll = 0;
    }
    if (scrollY > maxScroll)
    {
        scrollY = maxScroll;
    }
    if (scrollY < 0)
    {
        scrollY = 0;
    }
    putRect(x, y, w, h, pageBg);
    for (unsigned int i = 0; i < itemCount; i++)
    {
        const Item* it = &items[i];
        int iy = top + it->y - scrollY;
        if (iy < top || iy + lineH > bottom)
        {
            continue;
        }
        if (it->kind == 1)
        {
            putRect(x + it->x, iy + lineH / 2, it->w, 1, colRule);
        }
        else
        {
            drawWord(x + it->x, iy, it);
        }
    }
    if (maxScroll > 0)
    {
        int len = viewH * viewH / contentH;
        int pos;
        if (len < 8)
        {
            len = 8;
        }
        pos = scrollY * (viewH - len) / maxScroll;
        putRect(x + w - 4, top + pos, 4, len, colScroll);
    }
    putRect(x, y, w, barH, colBar);
    putRect(x + 4, y + 3, w - 8, barH - 6, colBg);
    drawStr(x + 6, y + 7, urlText + start, colText);
    putRect(x + 6 + (urlLen - start) * charW, y + 6, 2, 10, colText);
    putRect(x, bottom, w, statusH, colBar);
    drawStr(x + 4, bottom + 3, status, colText);
}

void browserClick(int x, int y)
{
    int py = y - barH + scrollY;
    if (x >= viewW - 12 && y >= barH)
    {
        browserScroll(y < barH + (viewH2 - barH - statusH) / 2 ? -5 : 5);
        return;
    }
    if (itemCount == 0)
    {
        return;
    }
    for (unsigned int i = 0; i < itemCount; i++)
    {
        const Item* it = &items[i];
        char target[urlMax];
        if (it->kind != 0 || it->link < 0)
        {
            continue;
        }
        if (x >= it->x && x < it->x + it->w && py >= it->y && py < it->y + lineH)
        {
            if (resolveLink(links[it->link], target))
            {
                browserOpen(target);
            }
            return;
        }
    }
}

void browserKey(int key)
{
    if (key == KEY_ESC)
    {
        if (state == brResolving || state == brConnecting || state == brLoading)
        {
            tcpAbort();
            fail("stopped");
        }
    }
    else if (key == KEY_ENTER)
    {
        browserOpen(urlText);
    }
    else if (key == KEY_BACKSPACE)
    {
        if (urlLen > 0)
        {
            urlText[--urlLen] = '\0';
        }
    }
    else if (key >= 32 && key < 127 && urlLen < urlMax - 1)
    {
        urlText[urlLen++] = (char)key;
        urlText[urlLen] = '\0';
    }
}

void browserScroll(int lines)
{
    scrollY += lines * lineH;
    if (scrollY < 0)
    {
        scrollY = 0;
    }
}

const char* browserTitle()
{
    return pageTitle[0] ? pageTitle : "browser";
}
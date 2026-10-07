#include "browser.h"
#include "../Graphics/gfx.h"
#include "../Network/dns.h"
#include "wm.h"
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
    dnsMaxTries = 4
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
    const char* text;
} Item;

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

static void buildOrigin(char* out, unsigned int max)
{
    unsigned int n = 0;
    brAppend(out, &n, max, "http://");
    brAppend(out, &n, max, host);
    if (port != 80)
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
    if (brPrefix(url, "https://"))
    {
        setStatus("https is not supported");
        return 0;
    }
    if (brPrefix(url, "http://"))
    {
        i = 7;
    }
    while (url[i] && url[i] != '/' && url[i] != ':' && url[i] != '?' && url[i] != '#' && h < sizeof(host) - 1)
    {
        host[h++] = url[i++];
    }
    host[h] = '\0';
    port = 80;
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
        brAppend(out, &n, urlMax, "http:");
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

static void sendRequest()
{
    char req[384];
    unsigned int n = 0;
    brAppend(req, &n, sizeof(req), "GET ");
    brAppend(req, &n, sizeof(req), path);
    brAppend(req, &n, sizeof(req), " HTTP/1.0\r\nHost: ");
    brAppend(req, &n, sizeof(req), host);
    if (port != 80)
    {
        brAppend(req, &n, sizeof(req), ":");
        brAppendNum(req, &n, sizeof(req), port);
    }
    brAppend(req, &n, sizeof(req), "\r\nUser-Agent: obos/0.1\r\nAccept: text/html\r\nConnection: close\r\n\r\n");
    tcpSend(req, n);
}

static void breakLine()
{
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
    if (itemCount < itemsMax && poolLen + wordLen + 1 < poolMax)
    {
        Item* it = &items[itemCount++];
        it->x = curX;
        it->y = curY;
        it->w = w;
        it->kind = 0;
        it->bold = bold || heading;
        it->link = curLink;
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
    else if (brEq(name, "p") || brEq(name, "ul") || brEq(name, "ol") || brEq(name, "table") || brEq(name, "blockquote") || brEq(name, "pre") || brEq(name, "form"))
    {
        blockBreak();
    }
    else if (brEq(name, "div") || brEq(name, "tr") || brEq(name, "dt") || brEq(name, "dd") || brEq(name, "section") || brEq(name, "article") || brEq(name, "header") || brEq(name, "footer"))
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
    else if (brEq(name, "script"))
    {
        return closing ? 0 : 1;
    }
    else if (brEq(name, "style"))
    {
        return closing ? 0 : 2;
    }
    else if (brEq(name, "title"))
    {
        return closing ? 4 : 3;
    }
    return 0;
}

static unsigned int entity(const char* p, const char* end, char* out)
{
    static const char* const names[] = {"amp;", "lt;", "gt;", "quot;", "nbsp;", "apos;"};
    static const char vals[] = {'&', '<', '>', '"', ' ', '\''};
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
        *out = (v >= 32 && v < 127) ? (char)v : '?';
        return i;
    }
    for (unsigned int k = 0; k < 6; k++)
    {
        if (brPrefix(p + 1, names[k]))
        {
            *out = vals[k];
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
                p = findNoCase(p, end, "</style");
            }
            else if (action == 3)
            {
                inTitle = 1;
            }
            else if (action == 4)
            {
                inTitle = 0;
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
    char msg[24];
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
    brAppend(msg, &n, sizeof(msg), "http ");
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
        if (tcpConnect(ip, port, onData, 0) != SUCCESS)
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
            sendRequest();
            lastActivity = ticks;
            setStatus("loading...");
            state = brLoading;
        }
        else if (ts == TCP_CLOSED)
        {
            fail("connection failed");
        }
        return;
    }
    if (ts == TCP_CLOSED)
    {
        finish();
    }
    else if (ticks - lastActivity > timeoutTicks)
    {
        tcpAbort();
        if (respLen > 0)
        {
            finish();
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
    unsigned int color = it->link >= 0 ? colLink : colText;
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
    putRect(x, y, w, h, colBg);
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
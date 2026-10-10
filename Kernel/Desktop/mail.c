#include "mail.h"
#include "../Graphics/gfx.h"
#include "../Drivers/timerDriver.h"
#include "../Network/dns.h"
#include "wm.h"
#include "../Crypto/tls.h"
#include "../SystemLib/util.h"
#include "../Network/tcp.h"

#define TOOLBAR_H 20
#define FOOTER_H 14
#define SIDEBAR_W 88
#define ROW_H 22

#define FOLDER_COUNT 4

/*
 * ---------------------------------------------------------------------
 *  SMTP settings. Change these to point at your mail server.
 *
 *  Testing under QEMU:  10.0.2.2 is your host machine, so run a local test
 *  server there (for example Mailpit, or `python3 -m aiosmtpd -n -l
 *  0.0.0.0:1025`) and leave the defaults below.
 *
 *  Real providers (Gmail, Outlook, ...) need TLS and a login: use port 465,
 *  SMTP_TLS 1 and fill in SMTP_USER / SMTP_PASS (an app password).
 *  Note: the TLS stack does not verify certificates yet.
 * ---------------------------------------------------------------------
 */
#ifndef SMTP_HOST
#define SMTP_HOST "10.0.2.2"
#endif
#ifndef SMTP_PORT
#define SMTP_PORT 1025
#endif
#ifndef SMTP_TLS
#define SMTP_TLS 0
#endif
#ifndef SMTP_USER
#define SMTP_USER ""
#endif
#ifndef SMTP_PASS
#define SMTP_PASS ""
#endif
#ifndef SMTP_FROM
#define SMTP_FROM "me@obos.local"
#endif
#ifndef SMTP_HELO
#define SMTP_HELO "obos.local"
#endif

enum
{
    toMax = 64,
    subjMax = 64,
    bodyMax = 1024,
    outMax = 4096,
    outChunk = 400,
    sentMax = 8,
    lineMax = 256,
    tlsRingSize = 16384,
    tlsChunk = 512,
    rxRingSize = 4096,
    dnsMaxTries = 4
};

#define SMTP_TIMEOUT (15 * TIMER_HZ)

static const char* folderNames[FOLDER_COUNT] = {
    "Inbox", "Sent", "Drafts", "Trash"
};

static int currentFolder = 0;
static int mailW = 420;
static int mailH = 300;

/* ------------------------------------------------------------------ */
/* small string helpers (no libc)                                      */
/* ------------------------------------------------------------------ */

static unsigned int mlLen(const char* s)
{
    unsigned int n = 0;
    while (s[n])
    {
        n++;
    }
    return n;
}

static void mlCopy(char* dst, unsigned int max, const char* s)
{
    unsigned int n = 0;
    while (*s && n < max - 1)
    {
        dst[n++] = *s++;
    }
    dst[n] = '\0';
}

static void mlAppend(char* dst, unsigned int max, const char* s)
{
    unsigned int n = mlLen(dst);
    while (*s && n < max - 1)
    {
        dst[n++] = *s++;
    }
    dst[n] = '\0';
}

static void mlAppendNum(char* dst, unsigned int max, unsigned int v)
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
        mlAppend(dst, max, one);
    }
}

static void mlBase64(const char* in, char* out, unsigned int max)
{
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned int len = mlLen(in);
    unsigned int n = 0;
    for (unsigned int i = 0; i < len; i += 3)
    {
        unsigned int v = (unsigned int)(unsigned char)in[i] << 16;
        if (i + 1 < len)
        {
            v |= (unsigned int)(unsigned char)in[i + 1] << 8;
        }
        if (i + 2 < len)
        {
            v |= (unsigned char)in[i + 2];
        }
        if (n + 5 >= max)
        {
            break;
        }
        out[n++] = tbl[(v >> 18) & 63];
        out[n++] = tbl[(v >> 12) & 63];
        out[n++] = (i + 1 < len) ? tbl[(v >> 6) & 63] : '=';
        out[n++] = (i + 2 < len) ? tbl[v & 63] : '=';
    }
    out[n] = '\0';
}

/* ------------------------------------------------------------------ */
/* footer status                                                       */
/* ------------------------------------------------------------------ */

static char statusBuf[64];
static unsigned int statusUntil;
static int statusSticky;

/* shown for two seconds */
static void setStatus(const char* msg)
{
    mlCopy(statusBuf, sizeof(statusBuf), msg);
    statusSticky = 0;
    statusUntil = getTicks() + 2 * TIMER_HZ;
}

/* stays until the user does something else */
static void setStickyStatus(const char* msg)
{
    mlCopy(statusBuf, sizeof(statusBuf), msg);
    statusSticky = 1;
}

/* ------------------------------------------------------------------ */
/* compose state and sent list                                         */
/* ------------------------------------------------------------------ */

static int composing;
static int focus; /* 0 = to, 1 = subject, 2 = body */
static char toText[toMax];
static char subjText[subjMax];
static char bodyText[bodyMax];
static unsigned int toLen;
static unsigned int subjLen;
static unsigned int bodyLen;

typedef struct
{
    char to[40];
    char subj[40];
} SentItem;

static SentItem sent[sentMax];
static unsigned int sentCount;

static void pushSent(void)
{
    unsigned int top = sentCount < sentMax ? sentCount : sentMax - 1;
    for (unsigned int i = top; i > 0; i--)
    {
        mlCopy(sent[i].to, sizeof(sent[i].to), sent[i - 1].to);
        mlCopy(sent[i].subj, sizeof(sent[i].subj), sent[i - 1].subj);
    }
    mlCopy(sent[0].to, sizeof(sent[0].to), toText);
    mlCopy(sent[0].subj, sizeof(sent[0].subj), subjText);
    if (sentCount < sentMax)
    {
        sentCount++;
    }
}

/* ------------------------------------------------------------------ */
/* SMTP client                                                         */
/* ------------------------------------------------------------------ */

typedef enum
{
    smIdle,
    smResolving,
    smConnecting,
    smGreeting,
    smEhlo,
    smAuth,
    smAuthUser,
    smAuthPass,
    smFrom,
    smRcpt,
    smData,
    smBody,
    smQuit,
    smDone,
    smError
} SmState;

static SmState sm = smIdle;
static char smtpHost[64];
static unsigned short smtpPort;
static unsigned char useTls;
static const char* authUser = SMTP_USER;
static const char* authPass = SMTP_PASS;
static unsigned int lastTick;
static unsigned int lastActivity;
static unsigned int dnsTries;
static unsigned int dnsSent;

static char outBuf[outMax];
static unsigned int outLen;
static unsigned int outPos;
static unsigned char outFull;

static char lineBuf[lineMax];
static unsigned int lineLen;

/* decrypted (or plain) server bytes: one writer, one reader */
static unsigned char rxRing[rxRingSize];
static volatile unsigned int rxHead;
static volatile unsigned int rxTail;
static volatile unsigned char rxOverflow;

/* raw tcp bytes while TLS is on */
static TlsConn conn;
static unsigned char tlsIn[tlsRingSize];
static volatile unsigned int tlsHead;
static volatile unsigned int tlsTail;
static volatile unsigned char tlsOverflow;

static int smBusy(void)
{
    return sm != smIdle && sm != smDone && sm != smError;
}

static void smtpFail(const char* msg)
{
    if (sm >= smConnecting)
    {
        tcpAbort();
    }
    sm = smError;
    setStickyStatus(msg);
}

/* lineBuf still holds the server's reply line, so the error shows what it said. */
static void smtpFailCode(const char* what, unsigned int code)
{
    char msg[64];
    mlCopy(msg, sizeof(msg), what);
    mlAppend(msg, sizeof(msg), ": ");
    if (lineBuf[0])
    {
        mlAppend(msg, sizeof(msg), lineBuf);
    }
    else
    {
        mlAppendNum(msg, sizeof(msg), code);
    }
    smtpFail(msg);
}

static const char* stageName(void)
{
    switch (sm)
    {
    case smResolving:
        return "finding server";
    case smConnecting:
        return "connecting";
    case smGreeting:
        return "greeting";
    case smEhlo:
        return "hello";
    case smAuth:
    case smAuthUser:
    case smAuthPass:
        return "login";
    case smFrom:
        return "sender";
    case smRcpt:
        return "recipient";
    case smData:
        return "data command";
    case smBody:
        return "message body";
    case smQuit:
        return "quit";
    default:
        return "?";
    }
}

static void rxPush(const unsigned char* data, unsigned int len)
{
    for (unsigned int i = 0; i < len; i++)
    {
        unsigned int next = (rxHead + 1) & (rxRingSize - 1);
        if (next == rxTail)
        {
            rxOverflow = 1;
            return;
        }
        rxRing[rxHead] = data[i];
        rxHead = next;
    }
    lastActivity = lastTick;
}

/* TCP receive callback: only queues bytes, the real work happens in mailPoll. */
static void onTcpData(const unsigned char* data, unsigned int len)
{
    if (!useTls)
    {
        rxPush(data, len);
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
    lastActivity = lastTick;
}

static void tlsSendOut(void* user, const unsigned char* data, unsigned int len)
{
    tcpSend((void*)data, len);
}

static void tlsGotData(void* user, const unsigned char* data, unsigned int len)
{
    rxPush(data, len);
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

static int outChar(char c)
{
    if (outLen >= outMax)
    {
        outFull = 1;
        return 0;
    }
    outBuf[outLen++] = c;
    return 1;
}

static void outAdd(const char* s)
{
    while (*s)
    {
        if (!outChar(*s++))
        {
            return;
        }
    }
}

static void cmd3(const char* a, const char* b, const char* c)
{
    outAdd(a);
    outAdd(b);
    outAdd(c);
    outAdd("\r\n");
}

/* Sends at most one chunk per call so a small tcp stack is never flooded. */
static void flushOut(TcpState ts)
{
    unsigned int n;
    if (outPos >= outLen)
    {
        return;
    }
    /* Only "closed" blocks sending: some tcp stacks report other states while data is in flight. */
    if (useTls ? tlsGetState(&conn) != tlsReady : ts == TCP_CLOSED)
    {
        return;
    }
    n = outLen - outPos;
    if (n > outChunk)
    {
        n = outChunk;
    }
    if (useTls)
    {
        tlsWrite(&conn, (const unsigned char*)outBuf + outPos, n);
    }
    else
    {
        tcpSend(outBuf + outPos, n);
    }
    outPos += n;
    lastActivity = lastTick;
    if (outPos >= outLen)
    {
        outPos = 0;
        outLen = 0;
    }
}

/* Headers + body of the message, dot-stuffed, ending with the "." line. */
static void queueMessage(void)
{
    int atStart = 1;
    outAdd("From: <");
    outAdd(SMTP_FROM);
    outAdd(">\r\nTo: <");
    outAdd(toText);
    outAdd(">\r\nSubject: ");
    outAdd(subjText);
    outAdd("\r\nMIME-Version: 1.0\r\nContent-Type: text/plain; charset=us-ascii\r\n\r\n");
    for (unsigned int i = 0; i < bodyLen; i++)
    {
        char c = bodyText[i];
        if (c == '\n')
        {
            outAdd("\r\n");
            atStart = 1;
        }
        else
        {
            if (atStart && c == '.')
            {
                outChar('.');
            }
            outChar(c);
            atStart = 0;
        }
    }
    if (!atStart)
    {
        outAdd("\r\n");
    }
    outAdd(".\r\n");
}

static void sendSucceeded(void)
{
    pushSent();
    toText[0] = '\0';
    subjText[0] = '\0';
    bodyText[0] = '\0';
    toLen = 0;
    subjLen = 0;
    bodyLen = 0;
    focus = 0;
    composing = 0;
    currentFolder = 1;
    setStatus("Message sent");
}

/* One complete server reply (final line) has arrived. */
static void handleReply(unsigned int code)
{
    char enc[96];
    switch (sm)
    {
    case smGreeting:
        if (code != 220)
        {
            smtpFailCode("server greeting", code);
            return;
        }
        cmd3("EHLO ", SMTP_HELO, "");
        sm = smEhlo;
        setStickyStatus("saying hello...");
        break;
    case smEhlo:
        if (code != 250)
        {
            smtpFailCode("hello rejected", code);
            return;
        }
        if (authUser[0])
        {
            cmd3("AUTH LOGIN", "", "");
            sm = smAuth;
            setStickyStatus("logging in...");
        }
        else
        {
            cmd3("MAIL FROM:<", SMTP_FROM, ">");
            sm = smFrom;
            setStickyStatus("sending...");
        }
        break;
    case smAuth:
        if (code != 334)
        {
            smtpFailCode("login not accepted", code);
            return;
        }
        mlBase64(authUser, enc, sizeof(enc));
        cmd3(enc, "", "");
        sm = smAuthUser;
        break;
    case smAuthUser:
        if (code != 334)
        {
            smtpFailCode("login failed", code);
            return;
        }
        mlBase64(authPass, enc, sizeof(enc));
        cmd3(enc, "", "");
        sm = smAuthPass;
        break;
    case smAuthPass:
        if (code != 235)
        {
            smtpFailCode("login failed", code);
            return;
        }
        cmd3("MAIL FROM:<", SMTP_FROM, ">");
        sm = smFrom;
        setStickyStatus("sending...");
        break;
    case smFrom:
        if (code != 250)
        {
            smtpFailCode("sender rejected", code);
            return;
        }
        cmd3("RCPT TO:<", toText, ">");
        sm = smRcpt;
        setStickyStatus("sending: recipient...");
        break;
    case smRcpt:
        if (code != 250 && code != 251)
        {
            smtpFailCode("recipient rejected", code);
            return;
        }
        cmd3("DATA", "", "");
        sm = smData;
        setStickyStatus("sending: data...");
        break;
    case smData:
        if (code != 354)
        {
            smtpFailCode("data rejected", code);
            return;
        }
        queueMessage();
        sm = smBody;
        setStickyStatus("sending: message...");
        break;
    case smBody:
        if (code != 250)
        {
            smtpFailCode("message rejected", code);
            return;
        }
        sendSucceeded();
        cmd3("QUIT", "", "");
        sm = smQuit;
        break;
    case smQuit:
        tcpAbort();
        sm = smDone;
        break;
    default:
        break;
    }
    if (outFull)
    {
        smtpFail("message too large");
    }
}

/* Splits the incoming bytes into lines and acts on each final reply line. */
static void parseReplies(void)
{
    while (rxTail != rxHead && smBusy())
    {
        char c = (char)rxRing[rxTail];
        rxTail = (rxTail + 1) & (rxRingSize - 1);
        if (c == '\n')
        {
            lineBuf[lineLen] = '\0';
            if (lineLen >= 3 && lineBuf[0] >= '0' && lineBuf[0] <= '9' && lineBuf[1] >= '0' && lineBuf[1] <= '9' && lineBuf[2] >= '0' && lineBuf[2] <= '9' && (lineLen == 3 || lineBuf[3] == ' '))
            {
                unsigned int code = (lineBuf[0] - '0') * 100 + (lineBuf[1] - '0') * 10 + (lineBuf[2] - '0');
                lineLen = 0;
                handleReply(code);
                continue;
            }
            lineLen = 0;
        }
        else if (c != '\r' && lineLen < lineMax - 1)
        {
            lineBuf[lineLen++] = c;
        }
    }
    if (rxOverflow && smBusy())
    {
        smtpFail("receive buffer overflow");
    }
}

static int pumpTls(void)
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
        smtpFail("tls receive buffer overflow");
        return 0;
    }
    if (tlsGetState(&conn) == tlsFailed)
    {
        smtpFail(tlsErrorText(&conn));
        return 0;
    }
    return 1;
}

static unsigned int dnsServerIp(void)
{
    return splitIP((unsigned char*)"10.0.2.3");
}

static unsigned int resolveHost(unsigned int ticks)
{
    unsigned int i = 0;
    unsigned int ip;
    int numeric = 1;
    while (smtpHost[i])
    {
        if (!((smtpHost[i] >= '0' && smtpHost[i] <= '9') || smtpHost[i] == '.'))
        {
            numeric = 0;
        }
        i++;
    }
    if (numeric)
    {
        ip = splitIP((unsigned char*)smtpHost);
        if (ip == 0)
        {
            smtpFail("bad server address");
        }
        return ip;
    }
    ip = dnsResolve(smtpHost);
    if (ip)
    {
        return ip;
    }
    if (dnsTries == 0 || ticks - dnsSent >= TIMER_HZ)
    {
        if (dnsTries >= dnsMaxTries)
        {
            smtpFail("dns lookup timed out");
            return 0;
        }
        dnsSendQuery(myIP, dnsServerIp(), smtpHost);
        dnsTries++;
        dnsSent = ticks;
        setStickyStatus("looking up server...");
    }
    return 0;
}

/*
 * Drives the whole send. Call it often (every frame / timer tick), also while
 * the mail window is not focused. The tcp stack only has one connection, so
 * don't send mail while the browser is loading a page.
 */
void mailPoll(unsigned int ticks)
{
    TcpState ts;
    int tick = ticks != lastTick;
    lastTick = ticks;
    if (!smBusy())
    {
        return;
    }
    if (sm == smResolving)
    {
        unsigned int ip = resolveHost(ticks);
        if (ip == 0)
        {
            return;
        }
        if (tcpConnect(ip, smtpPort, onTcpData, 0) != SUCCESS)
        {
            smtpFail("network busy, try again");
            return;
        }
        lastActivity = ticks;
        setStickyStatus("connecting...");
        sm = smConnecting;
        return;
    }
    if (tick)
    {
        tcpTick();
    }
    ts = tcpGetState();
    if (sm == smConnecting)
    {
        if (ts == TCP_ESTABLISHED)
        {
            if (useTls)
            {
                unsigned char rnd[64];
                fillRandom(rnd, 64);
                tlsInit(&conn, smtpHost, rnd, tlsSendOut, tlsGotData, 0);
                tlsStart(&conn);
                setStickyStatus("tls handshake...");
            }
            else
            {
                setStickyStatus("waiting for server...");
            }
            lastActivity = ticks;
            sm = smGreeting;
        }
        else if (ts == TCP_CLOSED)
        {
            smtpFail("connection failed");
        }
        return;
    }
    if (useTls && !pumpTls())
    {
        return;
    }
    parseReplies();
    if (!smBusy())
    {
        return;
    }
    flushOut(ts);
    if (ts == TCP_CLOSED)
    {
        if (sm == smQuit)
        {
            sm = smDone;
        }
        else
        {
            smtpFail("server closed connection");
        }
    }
    else if (ticks - lastActivity > SMTP_TIMEOUT)
    {
        if (sm == smQuit)
        {
            tcpAbort();
            sm = smDone;
        }
        else
        {
            char msg[64];
            mlCopy(msg, sizeof(msg), "no reply during ");
            mlAppend(msg, sizeof(msg), stageName());
            smtpFail(msg);
        }
    }
}

static int validAddr(const char* a)
{
    int at = -1;
    int n = 0;
    for (; a[n]; n++)
    {
        char c = a[n];
        if (c == ' ' || c == '<' || c == '>' || c == ',' || c == ';')
        {
            return 0;
        }
        if (c == '@')
        {
            if (at >= 0)
            {
                return 0;
            }
            at = n;
        }
    }
    return at > 0 && at < n - 1;
}

static void startSend(void)
{
    if (smBusy())
    {
        setStatus("still sending...");
        return;
    }
    if (!validAddr(toText))
    {
        setStatus("enter a valid recipient");
        focus = 0;
        return;
    }
    mlCopy(smtpHost, sizeof(smtpHost), SMTP_HOST);
    smtpPort = SMTP_PORT;
    useTls = SMTP_TLS;
    rxHead = 0;
    rxTail = 0;
    rxOverflow = 0;
    tlsHead = 0;
    tlsTail = 0;
    tlsOverflow = 0;
    outLen = 0;
    outPos = 0;
    outFull = 0;
    lineLen = 0;
    dnsTries = 0;
    lastActivity = lastTick;
    sm = smResolving;
    setStickyStatus("resolving...");
}

static void cancelSend(void)
{
    if (!smBusy())
    {
        return;
    }
    if (sm >= smConnecting)
    {
        tcpAbort();
    }
    sm = smIdle;
    setStatus("send cancelled");
}

/* ------------------------------------------------------------------ */
/* compose layout, drawing and input                                   */
/* ------------------------------------------------------------------ */

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} Rect;

static Rect rTo;
static Rect rSubj;
static Rect rBody;
static Rect rSend;
static Rect rCancel;

static void setRect(Rect* r, int x, int y, int w, int h)
{
    r->x = x;
    r->y = y;
    r->w = w < 8 ? 8 : w;
    r->h = h < 8 ? 8 : h;
}

/* Positions are relative to the mail window. */
static void layoutCompose(void)
{
    int cx = SIDEBAR_W;
    int cy = TOOLBAR_H;
    int cw = mailW - SIDEBAR_W;
    int ch = mailH - TOOLBAR_H - FOOTER_H;
    setRect(&rTo, cx + 48, cy + 26, cw - 56, 16);
    setRect(&rSubj, cx + 48, cy + 46, cw - 56, 16);
    setRect(&rBody, cx + 6, cy + 68, cw - 12, ch - 68 - 26);
    setRect(&rSend, cx + 6, cy + ch - 22, 52, 18);
    setRect(&rCancel, cx + 62, cy + ch - 22, 56, 18);
}

static int inRect(const Rect* r, int x, int y)
{
    return x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h;
}

static void drawBtn(int x, int y, int w, int h,
                    const char* label, Color textColor)
{
    Color gray = {192, 192, 192};
    Color light = {255, 255, 255};
    Color dark = {96, 96, 96};

    gfxFillRect(x, y, w, h, gray);
    gfxFillRect(x, y, w, 1, light);
    gfxFillRect(x, y, 1, h, light);
    gfxFillRect(x, y + h - 1, w, 1, dark);
    gfxFillRect(x + w - 1, y, 1, h, dark);

    gfxDrawString(label, x + 4, y + (h - 8) / 2, textColor);
}

static void drawBox(int x, int y, int w, int h, int active)
{
    Color white = {255, 255, 255};
    Color dark = {96, 96, 96};
    Color blue = {0, 0, 128};
    Color edge = active ? blue : dark;
    gfxFillRect(x, y, w, h, white);
    gfxFillRect(x, y, w, 1, edge);
    gfxFillRect(x, y, 1, h, edge);
    gfxFillRect(x, y + h - 1, w, 1, edge);
    gfxFillRect(x + w - 1, y, 1, h, edge);
}

static void drawLineField(int x, int y, const Rect* r, const char* text, unsigned int len, int active)
{
    Color black = {0, 0, 0};
    int cols = (r->w - 10) / 8;
    unsigned int start;
    if (cols < 1)
    {
        cols = 1;
    }
    start = len > (unsigned int)cols ? len - cols : 0;
    drawBox(x + r->x, y + r->y, r->w, r->h, active);
    gfxDrawString(text + start, x + r->x + 4, y + r->y + 4, black);
    if (active)
    {
        gfxFillRect(x + r->x + 4 + (len - start) * 8, y + r->y + 3, 2, 10, black);
    }
}

/*
 * Walks the body text word-wrapped to `cols` columns. When draw is set it
 * paints lines [skip, skip+rows). Returns the number of lines and the length
 * of the last one through lastLen.
 */
static int walkBody(int cols, int skip, int rows, int bx, int by, int draw, int* lastLen)
{
    Color black = {0, 0, 0};
    char buf[64];
    unsigned int i = 0;
    int line = 0;
    if (cols > 63)
    {
        cols = 63;
    }
    if (cols < 1)
    {
        cols = 1;
    }
    for (;;)
    {
        int n = 0;
        while (i < bodyLen && bodyText[i] != '\n' && n < cols)
        {
            buf[n++] = bodyText[i++];
        }
        buf[n] = '\0';
        if (draw && line >= skip && line - skip < rows)
        {
            gfxDrawString(buf, bx, by + (line - skip) * 12, black);
        }
        line++;
        *lastLen = n;
        if (i < bodyLen && bodyText[i] == '\n')
        {
            i++;
        }
        else if (i >= bodyLen)
        {
            break;
        }
    }
    return line;
}

static void drawCompose(int x, int y, int w, int h)
{
    Color black = {0, 0, 0};
    Color strip = {230, 230, 230};
    Color dark = {96, 96, 96};
    int contentX = x + SIDEBAR_W;
    int contentY = y + TOOLBAR_H;
    int contentW = w - SIDEBAR_W;
    int cols = (rBody.w - 10) / 8;
    int rows = (rBody.h - 6) / 12;
    int last = 0;
    int lines;
    int skip;

    if (rows < 1)
    {
        rows = 1;
    }
    gfxFillRect(contentX, contentY, contentW, 20, strip);
    gfxDrawString("New message", contentX + 6, contentY + 6, black);

    gfxDrawString("To:", contentX + 6, contentY + 30, black);
    gfxDrawString("Subj:", contentX + 6, contentY + 50, black);
    drawLineField(x, y, &rTo, toText, toLen, focus == 0);
    drawLineField(x, y, &rSubj, subjText, subjLen, focus == 1);

    drawBox(x + rBody.x, y + rBody.y, rBody.w, rBody.h, focus == 2);
    lines = walkBody(cols, 0, rows, 0, 0, 0, &last);
    skip = lines > rows ? lines - rows : 0;
    walkBody(cols, skip, rows, x + rBody.x + 4, y + rBody.y + 4, 1, &last);
    if (focus == 2)
    {
        gfxFillRect(x + rBody.x + 4 + last * 8, y + rBody.y + 3 + (lines - 1 - skip) * 12, 2, 10, black);
    }

    drawBtn(x + rSend.x, y + rSend.y, rSend.w, rSend.h, smBusy() ? "..." : "Send", smBusy() ? dark : black);
    drawBtn(x + rCancel.x, y + rCancel.y, rCancel.w, rCancel.h, smBusy() ? "Stop" : "Close", black);
}

static void drawFolder(int x, int y, int w, int h)
{
    Color black = {0, 0, 0};
    Color strip = {230, 230, 230};
    Color dark = {96, 96, 96};
    int contentX = x + SIDEBAR_W;
    int contentY = y + TOOLBAR_H;
    int contentW = w - SIDEBAR_W;
    int cols = (contentW - 16) / 8;

    gfxFillRect(contentX, contentY, contentW, 20, strip);
    gfxDrawString(folderNames[currentFolder], contentX + 6, contentY + 6, black);

    if (currentFolder == 1 && sentCount > 0)
    {
        for (unsigned int i = 0; i < sentCount; i++)
        {
            char line[64];
            int ry = contentY + 22 + (int)i * ROW_H;
            int n;
            if (ry + ROW_H > y + h - FOOTER_H)
            {
                break;
            }
            if (i % 2)
            {
                gfxFillRect(contentX, ry, contentW, ROW_H, strip);
            }
            mlCopy(line, sizeof(line), sent[i].to);
            mlAppend(line, sizeof(line), " - ");
            mlAppend(line, sizeof(line), sent[i].subj[0] ? sent[i].subj : "(no subject)");
            n = (int)mlLen(line);
            if (n > cols && cols > 0 && cols < 63)
            {
                line[cols] = '\0';
            }
            gfxDrawString(line, contentX + 8, ry + 7, black);
        }
    }
    else
    {
        /* No fake emails: only mail that was really sent is listed. */
        gfxDrawString("No messages", contentX + 12, contentY + 36, dark);
    }
}

void mailDraw(int x, int y, int w, int h)
{
    Color gray = {192, 192, 192};
    Color white = {255, 255, 255};
    Color black = {0, 0, 0};
    Color highlight = {0, 0, 128};
    Color red = {180, 0, 0};

    mailW = w;
    mailH = h;
    layoutCompose();

    /* Main background */
    gfxFillRect(x, y, w, h, white);

    /* Toolbar */
    gfxFillRect(x, y, w, TOOLBAR_H, gray);

    drawBtn(x + 4, y + 2, 68, 16, "Compose", black);
    drawBtn(x + 76, y + 2, 60, 16, "Refresh", black);

    /* Sidebar */
    gfxFillRect(x, y + TOOLBAR_H, SIDEBAR_W, h - TOOLBAR_H, gray);

    gfxDrawString("Folders", x + 6, y + TOOLBAR_H + 6, black);

    for (int i = 0; i < FOLDER_COUNT; i++)
    {
        int fy = y + TOOLBAR_H + 22 + i * ROW_H;

        if (i == currentFolder && !composing)
        {
            gfxFillRect(x + 3, fy - 2, SIDEBAR_W - 6,
                        ROW_H, highlight);
            gfxDrawString(folderNames[i], x + 8, fy + 3, white);
        }
        else
        {
            gfxDrawString(folderNames[i], x + 8, fy + 3, black);
        }
    }

    /* Main content area */
    gfxFillRect(x + SIDEBAR_W, y + TOOLBAR_H, w - SIDEBAR_W, h - TOOLBAR_H - FOOTER_H, white);

    if (composing)
    {
        drawCompose(x, y, w, h);
    }
    else
    {
        drawFolder(x, y, w, h);
    }

    /* Footer */
    gfxFillRect(x, y + h - FOOTER_H, w, FOOTER_H, gray);

    if (statusBuf[0] && (statusSticky || getTicks() < statusUntil))
        gfxDrawString(statusBuf, x + 4, y + h - FOOTER_H + 3, red);
    else
        gfxDrawString("Mail", x + 4, y + h - FOOTER_H + 3, black);
}

void mailClick(int lx, int ly)
{
    if (!smBusy())
    {
        statusSticky = 0;
    }

    if (ly < TOOLBAR_H)
    {
        if (ly < 2 || ly >= TOOLBAR_H - 2)
            return;

        if (lx >= 4 && lx < 72)
        {
            composing = 1;
        }
        else if (lx >= 76 && lx < 136)
        {
            setStatus("Receiving mail is not implemented");
        }

        return;
    }

    /* Folder selection */
    if (lx >= 0 && lx < SIDEBAR_W)
    {
        int folderY = TOOLBAR_H + 22;
        int index = (ly - folderY) / ROW_H;

        if (ly >= folderY &&
            index >= 0 && index < FOLDER_COUNT)
        {
            currentFolder = index;
            composing = 0; /* the draft is kept, Compose shows it again */
        }

        return;
    }

    if (composing)
    {
        layoutCompose();
        if (inRect(&rSend, lx, ly))
        {
            startSend();
        }
        else if (inRect(&rCancel, lx, ly))
        {
            if (smBusy())
                cancelSend();
            else
                composing = 0;
        }
        else if (!smBusy())
        {
            if (inRect(&rTo, lx, ly))
                focus = 0;
            else if (inRect(&rSubj, lx, ly))
                focus = 1;
            else if (inRect(&rBody, lx, ly))
                focus = 2;
        }
        return;
    }

    /* No messages exist to open, select, or delete yet. */
    if (lx >= SIDEBAR_W &&
        lx < mailW &&
        ly >= TOOLBAR_H &&
        ly < mailH - FOOTER_H)
    {
        setStatus("No message selected");
    }
}

void mailKey(int key)
{
    char* buf;
    unsigned int* len;
    unsigned int max;

    if (!composing)
    {
        return;
    }
    if (key == KEY_ESC)
    {
        if (smBusy())
            cancelSend();
        else
            composing = 0;
        return;
    }
    if (smBusy())
    {
        return;
    }
    statusSticky = 0;
    if (key == KEY_ENTER)
    {
        if (focus == 0)
        {
            focus = 1;
        }
        else if (focus == 1)
        {
            focus = 2;
        }
        else if (bodyLen < bodyMax - 1)
        {
            bodyText[bodyLen++] = '\n';
            bodyText[bodyLen] = '\0';
        }
        return;
    }
    if (focus == 0)
    {
        buf = toText;
        len = &toLen;
        max = toMax;
    }
    else if (focus == 1)
    {
        buf = subjText;
        len = &subjLen;
        max = subjMax;
    }
    else
    {
        buf = bodyText;
        len = &bodyLen;
        max = bodyMax;
    }
    if (key == KEY_BACKSPACE)
    {
        if (*len > 0)
        {
            buf[--*len] = '\0';
        }
    }
    else if (key >= 32 && key < 127 && *len < max - 1)
    {
        buf[(*len)++] = (char)key;
        buf[*len] = '\0';
    }
}
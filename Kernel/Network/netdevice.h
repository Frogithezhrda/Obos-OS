#ifndef NET_DEVICE_H
#define NET_DEVICE_H

#define MAX_PACKET_SIZE  1518
#define RX_BUFFER_COUNT  16
#define TX_BUFFER_COUNT  16
#define MAX_BUFFER_COUNT 16
#define ERROR -1
#define OK 0
#define NULL 0
typedef struct PacketBuffer {
    unsigned char data[MAX_PACKET_SIZE];
    unsigned int length;
    struct PacketBuffer* next;
} PacketBuffer;


typedef struct NetDeviceFunc
{
    int  (*init)  (struct NetDevice* dev);
    int  (*send)  (struct NetDevice* dev, PacketBuffer* pkt);
    void (*destroy)  (struct NetDevice* dev);  // for virtual/polling mode
    void (*irqHandler)(struct NetDevice* dev);  // for real hardware
} NetDeviceFunc;


typedef struct NetDevice {
    char name[MAX_BUFFER_COUNT];
    unsigned char mac[6];

    // queues
    PacketBuffer* rxHead;
    PacketBuffer* rxTail;
    PacketBuffer* txHead;
    PacketBuffer* txTail;
    unsigned int rxCount;
    unsigned int txCount;

    // stats
    unsigned int rxPackets;
    unsigned int txPackets;
    unsigned int rxDropped;
    unsigned int txDropped;

    NetDeviceFunc* ops;
    // internal state
    int initialized;
    void* driverData;
} NetDevice;

NetDevice* netDeviceCreate(const char* name, unsigned char mac[6]);
void netDeviceDestroy(NetDevice* dev);
int netDeviceInit(NetDevice* dev);
PacketBuffer* packetBufferAlloc();
void packetBufferFree(PacketBuffer* pkt);
int netDeviceTransmit(NetDevice* dev, PacketBuffer* pkt);
PacketBuffer* netDeviceReceive(NetDevice* dev);  // dequeues one rx packet

// interrupt
void netDeviceHandleIRQ(NetDevice* dev);
#endif // NET_DEVICE_H
#include "netdevice.h"

NetDevice* netDeviceCreate(const char* name, unsigned char mac[6])
{
    NetDevice* dev = (NetDevice*)kmalloc(sizeof(NetDevice));
    if (dev == NULL) return NULL;

    memset(dev, 0, sizeof(NetDevice));
    memcpy(dev->name, name, 15);
    memcpy(dev->mac, mac, 6);
    return dev;
}

void netDeviceDestroy(NetDevice* dev)
{
    if (dev == NULL) return;

    if (dev->ops && dev->ops->destroy)
    {
        dev->ops->destroy(dev);
    }

    PacketBuffer* pkt;
    while ((pkt = netDeviceReceive(dev)) != NULL)
    {
        packetBufferFree(pkt);
    }

    if (dev->driverData)
    {
        kfree(dev->driverData);
    }

    kfree(dev);
}

int netDeviceInit(NetDevice* dev)
{
    if (dev == NULL || dev->initialized)
        return ERROR;

    if (dev->ops && dev->ops->init)
    {
        if (dev->ops->init(dev) == ERROR)
        {
            return ERROR;
        }
    }

    dev->initialized = 1;
    return OK;
}

PacketBuffer* packetBufferAlloc()
{
    PacketBuffer* pkt = (PacketBuffer*)kmalloc(sizeof(PacketBuffer));
    if (pkt == NULL) return NULL;
    memset(pkt, 0, sizeof(PacketBuffer));
    return pkt;
}

void packetBufferFree(PacketBuffer* pkt)
{
    if (pkt == NULL) return;
    kfree(pkt);
}

void netDeviceEnqueueRX(NetDevice* dev, PacketBuffer* pkt)
{
    if (dev == NULL || pkt == NULL) return;

    if (dev->rxCount >= RX_BUFFER_COUNT)
    {
        dev->rxDropped++;
        packetBufferFree(pkt);
        return;
    }

    pkt->next = NULL;
    if (dev->rxTail)
    {
        dev->rxTail->next = pkt;
    }
    else
    {
        dev->rxHead = pkt;
    }

    dev->rxTail = pkt;
    dev->rxCount++;
    dev->rxPackets++;
}

PacketBuffer* netDeviceReceive(NetDevice* dev)
{
    if (dev == NULL || dev->rxHead == NULL) return NULL;

    PacketBuffer* pkt = dev->rxHead;
    dev->rxHead = pkt->next;
    if (dev->rxHead == NULL) dev->rxTail = NULL;
    dev->rxCount--;
    pkt->next = NULL;
    return pkt;
}

int netDeviceTransmit(NetDevice* dev, PacketBuffer* pkt)
{
    if (dev == NULL || pkt == NULL || !dev->initialized) return ERROR;

    if (dev->txCount >= TX_BUFFER_COUNT)
    {
        dev->txDropped++;
        return ERROR;
    }

    pkt->next = NULL;
    if (dev->txTail)
    {
        dev->txTail->next = pkt;
    }
    else
    {
        dev->txHead = pkt;
    }

    dev->txTail = pkt;
    dev->txCount++;

    if (dev->ops && dev->ops->send)
    {
        PacketBuffer* cur = dev->txHead;
        while (cur != NULL)
        {
            PacketBuffer* next = cur->next;
            if (dev->ops->send(dev, cur) == OK)
            {
                dev->txPackets++;
                packetBufferFree(cur);
                dev->txCount--;
            }
            else
            {
                dev->txHead = cur;
                if (dev->txHead == NULL) dev->txTail = NULL;
                return ERROR;
            }
            cur = next;
        }
        dev->txHead = dev->txTail = NULL;
    }

    return OK;
}

void netDeviceHandleIRQ(NetDevice* dev)
{
    if (dev == NULL || !dev->initialized) return;

    if (dev->ops && dev->ops->irqHandler)
    {
        dev->ops->irqHandler(dev);
    }
}
#include "netdevice.h"

//supporting only one device
static NetDevice device;

void netDeviceRegister(NetDevice* dev)
{
    device = *dev;
}

NetDevice* netDeviceGet(const char* name)
{
    if (strcmp(device.name, name) == 0)
    {
        return &device;
    }
    return 0;
}

int netDeviceSend(NetDevice* dev, const void* data, unsigned int len)
{
    if (!dev || !dev->send || !data || len == 0 || len > ETH_FRAME_MAX)
    {
        return ERROR;
    }
    printLineW("device sended");
    return dev->send(dev, data, len);
}

void netDeviceReceive(NetDevice* dev, const void* data, unsigned int len)
{
    printW("rece");
    if (dev && dev->rx && data && len > 0)
    {
        dev->rx(dev, data, len);
    }
}
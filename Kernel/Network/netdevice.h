#ifndef NET_DEVICE_H
#define NET_DEVICE_H

#include "../SystemLib/obosMemory.h"
#include "../Drivers/consoleDriver.h"

#define ETH_MAC_LEN 6
#define ETH_FRAME_MAX 1518
#define MAX_NAME_LEN 8

#define ERROR -1
#define SUCCESS 0

struct NetDevice;

//called by the driver when a packet is received
typedef void (*rxHandler)(struct NetDevice* dev,
                              const void* data, unsigned int len);
 
typedef struct NetDevice 
{
    char name[MAX_NAME_LEN];
    unsigned char mac[ETH_MAC_LEN];
    rxHandler rx;
    void* priv;
    //send packet for each netdevice
    int (*send)(struct NetDevice* dev, const void* data, unsigned int len);
} NetDevice;
 
void netDeviceRegister(NetDevice* dev);
NetDevice* netDeviceGet(const char* name);
 
int netDeviceSend(NetDevice* dev, const void* data, unsigned int len);
 
void netDeviceReceive(NetDevice* dev, const void* data, unsigned int len);
#endif // NET_DEVICE_H
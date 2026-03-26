#include "net.h"

static NetDevice* eth0;

void initializeNet()
{
    rtlInitialize();

    eth0 = netDeviceGet("eth0");

    ethernetInit(eth0);
    arpInit(eth0);
    ipInit(eth0);
    icmpInit(eth0);
}


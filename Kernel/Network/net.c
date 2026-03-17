#include "net.h"
#include "../SystemLib/errors.h"
static E1000Data e1000Data;
static NetDevice eth0;

void initializeNet()
{
    void* mmioBase = pciGetBar0(E1000_BUS, E1000_DEV, E1000_FUNC);
    if (!mmioBase)
    {
        kernelPanic("e1000 not found!");
        return;
    }
    e1000Init(mmioBase, &e1000Data, &eth0);
    for (volatile int i = 0; i < 10000000; i++);
    ethernetInit(&eth0);
    arpInit(&eth0);
    ipInit(&eth0);
    icmpInit(&eth0);
}
#include "e1000.h"

static NetDevice* e1000Dev = NULL;

// static unsigned int e1000Rd(E1000Data* d, unsigned int reg) 
// {
//     return *(volatile unsigned int*)(d->mmio + reg);
// }

// static void e1000Wr(E1000Data* d, unsigned int reg, unsigned int val) 
// {
//     *(volatile unsigned int*)(d->mmio + reg) = val;
// }

// static int e1000OpInit(NetDevice* dev)
// {
//     E1000Data* d = (E1000Data*)dev->driverData;

//     unsigned int ral = e1000Rd(d, E1000_RAL);
//     unsigned int rah = e1000Rd(d, E1000_RAH);
//     dev->mac[0] = (ral >>  0) & 0xFF;
//     dev->mac[1] = (ral >>  8) & 0xFF;
//     dev->mac[2] = (ral >> 16) & 0xFF;
//     dev->mac[3] = (ral >> 24) & 0xFF;
//     dev->mac[4] = (rah >>  0) & 0xFF;
//     dev->mac[5] = (rah >>  8) & 0xFF;

//     for (int i = 0; i < RX_DESC_COUNT; i++) 
//     {
//         d->rxDescs[i].addr   = (unsigned int)d->rxBuf[i];
//         d->rxDescs[i].status = 0;
//     }
//     e1000Wr(d, E1000_RDBAL, (unsigned int)d->rxDescs);
//     e1000Wr(d, E1000_RDBAH, 0);
//     e1000Wr(d, E1000_RDLEN, RX_DESC_COUNT * sizeof(RXDesc));
//     e1000Wr(d, E1000_RDH,   0);
//     e1000Wr(d, E1000_RDT,   RX_DESC_COUNT - 1);
//     e1000Wr(d, E1000_RCTL,  0x801E);

//     for (int i = 0; i < TX_DESC_COUNT; i++)
//     {
//         d->txDescs[i].status = 0x1;
//     }
//     e1000Wr(d, E1000_TDBAL, (unsigned int)d->txDescs);
//     e1000Wr(d, E1000_TDBAH, 0);
//     e1000Wr(d, E1000_TDLEN, TX_DESC_COUNT * sizeof(TXDesc));
//     e1000Wr(d, E1000_TDH,   0);
//     e1000Wr(d, E1000_TDT,   0);
//     e1000Wr(d, E1000_TCTL,  0x0003 | (1 << 3));

//     e1000Wr(d, E1000_IMS, ICR_RXT | ICR_RXO | ICR_LSC);

//     return OK;
// }

// static int e1000OpSend(NetDevice* dev, PacketBuffer* pkt) 
// {
//     E1000Data* d = (E1000Data*)dev->driverData;
//     int i = d->txTail;

//     if (!(d->txDescs[i].status & 0x1))
//         return ERROR;

//     memcpy(d->txBuf[i], pkt->data, pkt->length);
//     d->txDescs[i].addr   = (unsigned int)d->txBuf[i];
//     d->txDescs[i].length = pkt->length;
//     d->txDescs[i].cmd    = 0x3;
//     d->txDescs[i].status = 0;

//     d->txTail = (i + 1) % TX_DESC_COUNT;
//     e1000Wr(d, E1000_TDT, d->txTail);
//     return OK;
// }

// static void e1000OpIRQ(NetDevice* dev) 
// {
//     E1000Data* d = (E1000Data*)dev->driverData;

//     unsigned int icr = e1000Rd(d, E1000_ICR);

//     if (icr & ICR_RXT || icr & ICR_RXO) 
//     {
//         while (d->rxDescs[d->rxTail].status & 0x1) 
//         {
//             unsigned short len = d->rxDescs[d->rxTail].length;
//             unsigned char* buf = d->rxBuf[d->rxTail];

//             ethernetReceive(buf, len);

//             d->rxDescs[d->rxTail].status = 0;
//             d->rxTail = (d->rxTail + 1) % RX_DESC_COUNT;
//             e1000Wr(d, E1000_RDT, d->rxTail);
//         }
//     }

//     if (icr & ICR_LSC) 
//     {
//         unsigned int status = e1000Rd(d, E1000_STATUS);
//         (void)status;
//     }
// }

// static NetDeviceFunc e1000Ops = 
// {
//     .init       = e1000OpInit,
//     .send       = e1000OpSend,
//     .irqHandler = e1000OpIRQ,
//     .destroy    = NULL,
// };

void e1000ISRHandler()
{
    if (!e1000Dev) 
    {
        endOfInterrupt(11);
        return;
    }
    netDeviceHandleIRQ(e1000Dev);
    endOfInterrupt(11);
}

// void e1000Init(NetDevice* dev)
// {
//     if (!dev) return;

//     E1000Data* d = (E1000Data*)kmalloc(sizeof(E1000Data));
//     if (!d) return;
//     memset(d, 0, sizeof(E1000Data));

//     dev->driverData = d;
//     dev->ops = &e1000Ops;

//     e1000Dev = dev; 
// }
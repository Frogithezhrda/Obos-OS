#include "e1000.h"
static E1000Data* e1000Data = 0;
static NetDevice* e1000Dev = 0;
// read a 4 byte register from the card
static unsigned int regRead(E1000Data* hw, unsigned int reg)
{
    volatile unsigned int* addr = (volatile unsigned int*)(hw->mmio + reg);
    return *addr;
}
// write a 4 byte register to the card
static void regWrite(E1000Data* hw, unsigned int reg, unsigned int val)
{
    volatile unsigned int* addr = (volatile unsigned int*)(hw->mmio + reg);
    *addr = val;
}
// read one 16 bit word from the eeprom at the given index
static unsigned short eepromRead(E1000Data* hw, unsigned char index)
{
    regWrite(hw, E1000_EERD, 1 | ((unsigned int)index << 8));
    unsigned int result;
    do
    {
        result = regRead(hw, E1000_EERD);
    } while (!(result & (1u << 4)));
    return (unsigned short)(result >> 16);
}
// copy the mac address out of the eeprom into mac[6]
static void readMAC(E1000Data* hw, unsigned char* mac)
{
    unsigned short w0 = eepromRead(hw, 0);
    unsigned short w1 = eepromRead(hw, 1);
    unsigned short w2 = eepromRead(hw, 2);
    mac[0] = w0 & 0xFF;
    mac[1] = w0 >> 8;
    mac[2] = w1 & 0xFF;
    mac[3] = w1 >> 8;
    mac[4] = w2 & 0xFF;
    mac[5] = w2 >> 8;
}

static void txInit(E1000Data* hw)
{
    for (int i = 0; i < TX_DESC_COUNT; i++)
    {
        hw->txDescs[i].addr = 0;
        hw->txDescs[i].length = 0;
        hw->txDescs[i].cmd = 0;
        hw->txDescs[i].status = E1000_TXD_DD;
    }
    hw->txTail = 0;
    unsigned long long base = (unsigned long long)(unsigned long)hw->txDescs;
    regWrite(hw, E1000_TDBAL, (unsigned int)base);
    regWrite(hw, E1000_TDBAH, (unsigned int)(base >> 32));
    regWrite(hw, E1000_TDLEN, TX_DESC_COUNT * sizeof(TXDesc));
    regWrite(hw, E1000_TDH, 0);
    regWrite(hw, E1000_TDT, 0);
    regWrite(hw, E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP | (0x10u << 4) | (0x40u << 12));
}
static void rxInit(E1000Data* hw)
{
    for (int i = 0; i < RX_DESC_COUNT; i++)
    {
        hw->rxDescs[i].addr = (unsigned long long)(unsigned long)hw->rxBuf[i];
        hw->rxDescs[i].status = 0;
    }
    hw->rxTail = 0;
    unsigned long long base = (unsigned long long)(unsigned long)hw->rxDescs;
    regWrite(hw, E1000_RDBAL, (unsigned int)base);
    regWrite(hw, E1000_RDBAH, (unsigned int)(base >> 32));
    regWrite(hw, E1000_RDLEN, RX_DESC_COUNT * sizeof(RXDesc));
    regWrite(hw, E1000_RDH, 0);
    regWrite(hw, E1000_RDT, RX_DESC_COUNT - 1);
    regWrite(hw, E1000_RCTL, E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_SECRC);
}
static int e1000Send(NetDevice* dev, const void* data, unsigned int len)
{
    E1000Data* hw = (E1000Data*)dev->priv;
    int idx = hw->txTail;
    TXDesc* d = &hw->txDescs[idx]; 
    if (!(d->status & E1000_TXD_DD))
    {
        return ERROR;
    }
    memcpy(hw->txBuf[idx], (const unsigned char*)data, len);
    d->addr = (unsigned long long)(unsigned long)hw->txBuf[idx];
    d->length = (unsigned short)len;
    d->cmd = E1000_TXD_CMD_EOP | E1000_TXD_CMD_FCS | E1000_TXD_CMD_RS;
    d->status = 0;
    hw->txTail = (idx + 1) % TX_DESC_COUNT;
    regWrite(hw, E1000_TDT, (unsigned int)hw->txTail);
    return SUCCESS;
}
// check every rx slot for an arrived packet and pass it up
static void rxPoll()
{
    E1000Data* hw = (E1000Data*)e1000Dev->priv;
    while (1)
    {
        int idx = hw->rxTail;
        RXDesc* d = &hw->rxDescs[idx];
        if (!(d->status & E1000_RXD_DD))
        {
            break;
        }
        if (!d->errors && d->length > 0)
        {
            netDeviceReceive(e1000Dev, hw->rxBuf[idx], d->length);
        }
        d->addr = (unsigned long long)(unsigned long)hw->rxBuf[idx];
        d->status = 0;
        regWrite(hw, E1000_RDT, (unsigned int)idx);
        hw->rxTail = (idx + 1) % RX_DESC_COUNT;
    }
}
// read ICR to find out what happened then handle it (reading ICR clears it)
void e1000ISRHandler()
{
    printLineW("rep");
    if (!e1000Dev)
    {
        endOfInterrupt(11);
        return;
    }
    E1000Data* hw = (E1000Data*)e1000Dev->priv;
    unsigned int icr = regRead(hw, E1000_ICR);
    if (icr & E1000_ICR_RXT0)
    {
        rxPoll();
    }
    if (icr & E1000_ICR_LSC)
    {
        (void)regRead(hw, E1000_STATUS);
    }
    endOfInterrupt(11);
}
// reset the card, read the mac, set up tx and rx rings, register the device
void e1000Init(void* mmioBase, E1000Data* data, NetDevice* dev)
{
    e1000Data = data;
    e1000Dev = dev;
    data->mmio = (volatile unsigned char*)mmioBase;
    dev->priv  = data;
    dev->send  = e1000Send;
    regWrite(data, E1000_CTRL, E1000_CTRL_RST);
    regRead(data, E1000_STATUS);
    regWrite(data, E1000_CTRL, E1000_CTRL_SLU | E1000_CTRL_ASDE);
    unsigned int ctrl = regRead(data, E1000_CTRL);
    ctrl |= (1u << 6);
    regWrite(data, E1000_CTRL, ctrl);
    readMAC(data, dev->mac);
    unsigned int ral = (unsigned int)dev->mac[0]
                     | (unsigned int)dev->mac[1] <<  8
                     | (unsigned int)dev->mac[2] << 16
                     | (unsigned int)dev->mac[3] << 24;
    unsigned int rah = (unsigned int)dev->mac[4]
                     | (unsigned int)dev->mac[5] <<  8
                     | (-2147483648u); // set the high bit of RAH to indicate the address is valid
    regWrite(data, E1000_RAL, ral);
    regWrite(data, E1000_RAH, rah);
    txInit(data);
    rxInit(data);
    regWrite(data, E1000_IMS, E1000_ICR_RXT0 | E1000_ICR_TXDW | E1000_ICR_LSC);
    netDeviceRegister(dev);
}
void e1000SendRaw()
{
    if (!e1000Dev) return;
    E1000Data* hw = (E1000Data*)e1000Dev->priv;

    // hardcoded arp request packet
    // dst: ff:ff:ff:ff:ff:ff  src: 52:54:00:12:34:56
    // who has 10.0.2.2? tell 10.0.2.15
unsigned char test[60] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                           0x52,0x54,0x00,0x12,0x34,0x56,
                           0x08,0x06};
TXDesc* d = &hw->txDescs[0];
d->addr   = (unsigned long long)(unsigned long)test;
d->length = 60;
d->cmd    = E1000_TXD_CMD_EOP | E1000_TXD_CMD_FCS | E1000_TXD_CMD_RS;
d->status = 0;
regWrite(hw, E1000_TDT, 1);
for (volatile int i = 0; i < 1000000; i++);}
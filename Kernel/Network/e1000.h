#ifndef E1000_H
#define E1000_H

#include "netdevice.h"

#define E1000_STATUS 0x0008
#define E1000_RDBAL  0x2800
#define E1000_RDBAH  0x2804
#define E1000_RDLEN  0x2808
#define E1000_RDH    0x2810
#define E1000_RDT    0x2818
#define E1000_RCTL   0x0100
#define E1000_TDBAL  0x3800
#define E1000_TDBAH  0x3804
#define E1000_TDLEN  0x3808
#define E1000_TDH    0x3810
#define E1000_TDT    0x3818
#define E1000_TCTL   0x0400
#define E1000_RAL    0x5400
#define E1000_RAH    0x5404
#define E1000_ICR    0x00C0
#define E1000_IMS    0x00D0
#define E1000_IMC    0x00D8
#define E1000_VENDOR_ID  0x8086
#define E1000_DEVICE_ID  0x100E

#define ICR_TXDW   (1 << 0)
#define ICR_TXQE   (1 << 1)
#define ICR_LSC    (1 << 2)
#define ICR_RXO    (1 << 6)
#define ICR_RXT    (1 << 7)

#define RX_DESC_COUNT 16
#define TX_DESC_COUNT 16

typedef struct {
    unsigned long long addr;
    unsigned short length;
    unsigned short checksum;
    unsigned char status;
    unsigned char errors;
    unsigned short special;
} __attribute__((packed)) RXDesc;

typedef struct {
    unsigned long long addr;
    unsigned short length;
    unsigned char cso;
    unsigned char cmd;
    unsigned char status;
    unsigned char css;
    unsigned short special;
} __attribute__((packed)) TXDesc;

typedef struct {
    volatile unsigned char* mmio;

    RXDesc rxDescs[RX_DESC_COUNT] __attribute__((aligned(16)));
    TXDesc txDescs[TX_DESC_COUNT] __attribute__((aligned(16)));
    unsigned char rxBuf[RX_DESC_COUNT][MAX_PACKET_SIZE];
    unsigned char txBuf[TX_DESC_COUNT][MAX_PACKET_SIZE];

    int rxTail;
    int txTail;
} E1000Data;

void e1000Init(NetDevice* dev);
void e1000ISRHandler();

#endif
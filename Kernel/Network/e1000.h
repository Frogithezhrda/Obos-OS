#ifndef E1000_H
#define E1000_H

#include "netdevice.h"

#define E1000_VENDOR_ID  0x8086
#define E1000_DEVICE_ID  0x100E
 
/* ── MMIO register offsets ────────────────────────────────────── */
#define E1000_CTRL   0x0000
#define E1000_STATUS 0x0008
#define E1000_EERD   0x0014
 
#define E1000_ICR    0x00C0
#define E1000_IMS    0x00D0
#define E1000_IMC    0x00D8
 
#define E1000_RCTL   0x0100
#define E1000_RDBAL  0x2800
#define E1000_RDBAH  0x2804
#define E1000_RDLEN  0x2808
#define E1000_RDH    0x2810
#define E1000_RDT    0x2818
 
#define E1000_TCTL   0x0400
#define E1000_TDBAL  0x3800
#define E1000_TDBAH  0x3804
#define E1000_TDLEN  0x3808
#define E1000_TDH    0x3810
#define E1000_TDT    0x3818
 
#define E1000_RAL    0x5400
#define E1000_RAH    0x5404
/* ── CTRL bits ────────────────────────────────────────────────── */
#define E1000_CTRL_RST  67108864
#define E1000_CTRL_SLU  64
#define E1000_CTRL_ASDE 32

/* ── RCTL bits ────────────────────────────────────────────────── */
#define E1000_RCTL_EN    2
#define E1000_RCTL_BAM   32768
#define E1000_RCTL_SECRC 67108864

/* ── TCTL bits ────────────────────────────────────────────────── */
#define E1000_TCTL_EN  2
#define E1000_TCTL_PSP 8

/* ── ICR / IMS bits ───────────────────────────────────────────── */
#define E1000_ICR_TXDW 1
#define E1000_ICR_LSC  4
#define E1000_ICR_RXT0 128

/* ── TX descriptor command byte ───────────────────────────────── */
#define E1000_TXD_CMD_EOP 1
#define E1000_TXD_CMD_FCS 2
#define E1000_TXD_CMD_RS  8

/* ── Descriptor status byte ───────────────────────────────────── */
#define E1000_TXD_DD 1
#define E1000_RXD_DD 1
 
/* ── Ring sizes ───────────────────────────────────────────────── */
#define RX_DESC_COUNT 16
#define TX_DESC_COUNT 16
  
typedef struct RXDesc {
    unsigned long long addr;
    unsigned short length;
    unsigned short checksum;
    unsigned char status;
    unsigned char errors;
    unsigned short special;
} __attribute__((packed)) RXDesc;
 
typedef struct TXDesc {
    unsigned long long addr;
    unsigned short length;
    unsigned char cso;
    unsigned char cmd;
    unsigned char status;
    unsigned char css;
    unsigned short special;
} __attribute__((packed)) TXDesc;
 
 
typedef struct E1000Data {
    volatile unsigned char* mmio;
 
    RXDesc rxDescs[RX_DESC_COUNT] __attribute__((aligned(16)));
    unsigned char rxBuf [RX_DESC_COUNT][ETH_FRAME_MAX];
    int rxTail;
 
    TXDesc txDescs[TX_DESC_COUNT] __attribute__((aligned(16)));
    unsigned char txBuf [TX_DESC_COUNT][ETH_FRAME_MAX];
    int txTail;
} E1000Data;
 
/*
 * e1000Init  —  initialise the card and register it as a NetDevice.
 *
 *   mmioBase : virtual address of BAR0 (from your PCI code)
 *   data     : caller-allocated E1000Data, zeroed before calling
 *   dev      : caller-allocated NetDevice  (set dev->name first)
 */
void e1000Init(void *mmioBase, E1000Data *data, NetDevice *dev);
 
void e1000ISRHandler();
void e1000SendRaw();

#endif
 
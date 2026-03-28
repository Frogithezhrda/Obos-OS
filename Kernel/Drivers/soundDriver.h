#ifndef SOUND_DRIVER
#define SOUND_DRIVER
// Simple Sound Blaster 16 driver

#include "../Tables/PIC.h"

#define SB16_BASE 0x220
#define SB16_RESET (SB16_BASE + 0x6)
#define SB16_READ (SB16_BASE + 0xA)
#define SB16_WRITE (SB16_BASE + 0xC)
#define SB16_STATUS (SB16_BASE + 0xE)

#define SB16_IRQ 5
#define SB16_DMA 1

void soundBlasterInit();
void playSound(char* samples, unsigned int length, unsigned int rate);
void stopSound();

#endif
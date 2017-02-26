#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/tpiu.h>
#include <libopencm3/cm3/itm.h>

int _write(int file, char *ptr, int len);

/**
 * Trace on PB3
 */
static void trace_send_blocking(char c) {
    while (!(ITM_STIM8(0) & ITM_STIM_FIFOREADY));
    ITM_STIM8(0) = c;
}

int _write(int fp, char *c, int len) {
    (void) fp;
    int i;
    for (i = 0; i < len; i++) {
        if (c[i] == '\n') trace_send_blocking('\r');
        trace_send_blocking(c[i]);
    }
    return i;
}

static void delay_ms(int ms) {
    while(ms--) {
        for (uint32_t i = 0; i < (24000000UL / 4000) * 2; i++) {
            __asm__("nop");
        }
    }
}

static void delay_us(int us) {
    for (int i = 0; i < 7 * us; i++)
        __asm__("nop");
}

#endif /* UTIL_H */


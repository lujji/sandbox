#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>
#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/tpiu.h>
#include <libopencm3/cm3/itm.h>

int _write(int file, char *ptr, int len);

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

#endif /* TRACE_H */


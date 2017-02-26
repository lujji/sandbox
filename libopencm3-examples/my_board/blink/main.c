#include <stdio.h>
#include <string.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

// PA9
#define LED_PIN         GPIO13

enum REG_ID {
#define X(a,b) ID_##a,
    #include "xmacros.x"
#undef X
};

enum REG {
#define X(a,b) a = b,
    #include "xmacros.x"
#undef X
};

int regs[] = {
#define X(a,b) b,
    #include "xmacros.x"
#undef X
};

void write_reg(enum REG reg, int value) {
    int i = CMD1;

    regs[ID_CMD1] = value;
    regs[reg] = value;
}

static void delay_ms(int ms) {
    for (int j = 0; j < ms; j++)
        for (int i = 0; i < 400000; i++) /* Wait a bit. */
            __asm__("nop");
}

volatile int q = 0;

void main() {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
        GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
        GPIO_CNF_OUTPUT_PUSHPULL, GPIO15);

    gpio_set(GPIOC, GPIO15);
    for (int i=0; i<6; i++) {
        gpio_toggle(GPIOC, LED_PIN);
        delay_ms(10);
    }
    //gpio_clear(GPIOC, GPIO15);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
        GPIO_CNF_OUTPUT_OPENDRAIN, GPIO15);

    while (1) {

    }
}

#include <stdio.h>
#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/spi.h>

#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/tpiu.h>
#include <libopencm3/cm3/itm.h>

#include "lib/LCD_Protocol.h"

#define LED_PIN         GPIO13

int _write(int fp, char *c, int len);

static void clock_setup(void) {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup(void) {
    /* Set GPIO12 on Port C) to 'output push-pull' */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
}

static void delay_ms(int ms) {
    for (int j = 0; j < ms; j++)
        for (int i = 0; i < 40000; i++)
            __asm__("nop");
}

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

static void spi_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_SPI1);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL,
                  GPIO1 |
                  GPIO2 |
                  GPIO3);

    /* Configure GPIOs: SS=PA4, SCK=PA5, MISO=PA6 and MOSI=PA7 */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
            GPIO4 | // CS
            GPIO5 |
            GPIO6 |
            GPIO7);

    //gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO6);

    /* Reset SPI, SPI_CR1 register cleared, SPI is disabled */
    spi_reset(SPI1);

    /* Set up SPI in Master mode with:
     * Clock baud rate: 1/64 of peripheral clock frequency
     * Clock polarity: Idle High
     * Clock phase: Data valid on 2nd clock pulse
     * Data frame format: 8-bit
     * Frame format: MSB First
     */
    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_64, SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
            SPI_CR1_CPHA_CLK_TRANSITION_2, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

    /*
     * Set NSS management to software.
     *
     * Note:
     * Setting nss high is very important, even if we are controlling the GPIO
     * ourselves this bit needs to be at least set to 1, otherwise the spi
     * peripheral will not send any data out.
     */
    spi_enable_software_slave_management(SPI1);
    spi_set_nss_high(SPI1);

    /* Enable SPI1 periph. */
    spi_enable(SPI1);
}

static void putsh(int col) {
    //LCD_goto(0, 2);
    unsigned char txt[] = "Hello, World! ";
    //for (int i=0; i<col; i++)
        //LCD_writeChar(' ');
    for (uint8_t i = 0; i < sizeof(txt) - 1; i++)
        LCD_writeChar(txt[i]);
}

int main(void) {
    clock_setup();
    gpio_setup();
    gpio_set(GPIOC, LED_PIN);

    spi_setup();
    LCD_initialize();
    uint8_t counter = 0;

    for (int i = 0; i < 128; i++) {
        putsh(i);
        delay_ms(200);
    }
    while (1) {
        //LCD_writeChar('@');
        int reg = 42;
        //printf("tracingg: %d\n", reg);
        counter++;
        gpio_toggle(GPIOC, LED_PIN);
        delay_ms(100);
    }
}

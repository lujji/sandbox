/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>,
 * Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>

#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/tpiu.h>
#include <libopencm3/cm3/itm.h>

#define LED_PIN         GPIO13

int _write(int fp, char *c, int len);
static void delay_ms(int ms);

static void clock_setup(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    /* Enable GPIOC clock. */
    rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup(void)
{
    /* Set GPIO6/7/8/9 (in GPIO port C) to 'output push-pull'. */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
}

static void systick_setup()
{
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
    /* SysTick interrupt every N clock pulses: set reload to N-1 */
    systick_set_reload(4000000);
    systick_interrupt_enable();
    systick_counter_enable();
}

static void trace_send_blocking(char c)
{
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
    for (int j = 0; j < ms; j ++)
        for (int i = 0; i < 4000000; i++) /* Wait a bit. */
                __asm__("nop");
}

/************************ I2C ********************************/

static void i2c_setup(void) {
    /* Enable clocks for I2C2 and AFIO. */
    rcc_periph_clock_enable(RCC_I2C2);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_GPIOB);

    /* Set alternate functions for the SCL and SDA pins of I2C2. */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
            GPIO_I2C2_SCL | GPIO_I2C2_SDA);

    /* Disable the I2C before changing any configuration. */
    i2c_peripheral_disable(I2C2);

    /* APB1 is running at 36MHz. */
    i2c_set_clock_frequency(I2C2, I2C_CR2_FREQ_36MHZ);

    /* 400KHz - I2C Fast Mode */
    i2c_set_fast_mode(I2C2);

    /*
     * fclock for I2C is 36MHz APB2 -> cycle time 28ns, low time at 400kHz
     * incl trise -> Thigh = 1600ns; CCR = tlow/tcycle = 0x1C,9;
     * Datasheet suggests 0x1e.
     */
    i2c_set_ccr(I2C2, 0x1e);

    /*
     * fclock for I2C is 36MHz -> cycle time 28ns, rise time for
     * 400kHz => 300ns and 100kHz => 1000ns; 300ns/28ns = 10;
     * Incremented by 1 -> 11.
     */
    i2c_set_trise(I2C2, 0x0b);

    /*
     * This is our slave address - needed only if we want to receive from
     * other masters.
     */
    i2c_set_own_7bit_slave_address(I2C2, 0x32);

    /* If everything is configured -> enable the peripheral. */
    i2c_peripheral_enable(I2C2);
}

static void i2c_start(void) {
    uint32_t i2c = I2C2;
    i2c_send_start(i2c);
    /* Waiting for START is send and switched to master mode. */
    while (!((I2C_SR1(i2c) & I2C_SR1_SB) & (I2C_SR2(i2c) & (I2C_SR2_MSL | I2C_SR2_BUSY))));
    //while (!(I2C_SR1(i2c) & I2C_SR1_SB));
}

static void i2c_stop(void) {
    uint32_t i2c = I2C2;
    i2c_send_stop(i2c);
}

static void i2c_write_addr(uint8_t addr, int rw) {
    uint32_t i2c = I2C2;
    /* Say to what address we want to talk to. */
    i2c_send_7bit_address(i2c, addr, rw);
    /* Waiting for address is transferred. */
    while (!(I2C_SR1(i2c) & I2C_SR1_ADDR));
    /* Cleaning ADDR condition sequence. */
    (void) I2C_SR2(i2c);
}

static uint8_t i2c_read(int ack) {
    uint32_t i2c = I2C2;
    if (ack) i2c_enable_ack(i2c);
    else i2c_disable_ack(i2c);
    while (!(I2C_SR1(i2c) & I2C_SR1_BTF));
    return i2c_get_data(i2c);
}

static void i2c_read_arr(uint8_t *buf, int len) {
    int i2c = I2C2;

    i2c_enable_ack(i2c);
    while (len > 2) {
        /* Read into buffer */
        while (!(I2C_SR1(i2c) & I2C_SR1_RxNE));
        *(buf++) = i2c_get_data(i2c);
        len--;
    }

    /* Read last 2 bytes */
    while (!(I2C_SR1(i2c) & I2C_SR1_RxNE)); // should be BTF?
    i2c_disable_ack(i2c);
    i2c_stop();
    *(buf++) = i2c_get_data(i2c);


    while (!(I2C_SR1(i2c) & I2C_SR1_RxNE));
    *buf = i2c_get_data(i2c);

    i2c_enable_ack(i2c);
}

static void i2c_write(uint8_t data) {
    uint32_t i2c = I2C2;
    i2c_send_data(i2c, data);
    while (!(I2C_SR1(i2c) & (I2C_SR1_BTF | I2C_SR1_TxE)));
}

//#define DEV_ADDR        (0x1E << 1)
#define DEV_ADDR        0x1E
#define CR_A            0x00
#define CR_B            0x01
#define MODE            0x02
#define DATA_OUT        0x03
#define ID_REG_A        0x0A

uint8_t id[3];

static void get_id(void) {
    uint32_t i2c = I2C2;

    i2c_start();
    i2c_write_addr(DEV_ADDR, I2C_WRITE);
    i2c_write(ID_REG_A);
    i2c_stop();

    /*
     * Now we send another START condition (repeated START) and then
     * transfer the destination but with flag READ.
     */

    i2c_start();
    i2c_write_addr(DEV_ADDR, I2C_READ);
    i2c_read_arr(id, 3);

//     i2c_enable_ack(i2c);
//     while (!(I2C_SR1(i2c) & I2C_SR1_RxNE));
//     id[0] = i2c_get_data(i2c);
//
//     while (!(I2C_SR1(i2c) & I2C_SR1_RxNE));
//     i2c_disable_ack(i2c);
//     i2c_stop();
//
//     id[1] = i2c_get_data(i2c);
//
//     while (!(I2C_SR1(i2c) & I2C_SR1_RxNE));
//     id[2] = i2c_get_data(i2c);
//     /* Wait until STOP if cleared */
//     i2c_enable_ack(i2c);
}

int x, y, z;
#include <math.h>
#define RAD_TO_DEG 57.296

void measure() {
    float heading;
    int16_t hh;
    /* GAIN */
    i2c_start();
    i2c_write_addr(DEV_ADDR, I2C_WRITE);
    i2c_write(CR_B);
    i2c_write(0xe0);
    i2c_stop();

    /* Set cont. measurement mode */
    i2c_start();
    i2c_write_addr(DEV_ADDR, I2C_WRITE);
    i2c_write(MODE);
    i2c_write(0x00);
    i2c_stop();

    delay_ms(1); // 6

    /* Start reading @ DATA_OUT */
    i2c_start();
    i2c_write_addr(DEV_ADDR, I2C_WRITE);
    i2c_write(DATA_OUT);
    i2c_stop();

    /* Read axis */
    i2c_start();
    i2c_write_addr(DEV_ADDR, I2C_READ);
    uint8_t buf[6];
    i2c_read_arr(buf, 6);

    x = (buf[0] << 8) | buf[1];
    z = (buf[2] << 8) | buf[3];
    y = (buf[4] << 8) | buf[5];

    //i2c_stop();

    printf("x: %d y: %d z: %d\n", (int16_t) x, (int16_t)y, (int16_t)z);

//     heading = atan2f(y*4.35, x*4.35);
//     float PI = 3.14159;

//    //Correct for when signs are reversed.
//     if(heading < 0) heading += 2*PI;
//     if(heading > 2*PI) heading -= 2*PI;
//     hh = (heading*RAD_TO_DEG);
//     printf("deg :: %d\n", hh);
}

void sys_tick_handler(void) {
    gpio_toggle(GPIOC, LED_PIN);
    //get_id();
    //printf("ID: %c%c%c\n", id[0], id[1], id[2]);
    measure();
}

int main(void)
{
    clock_setup();
    gpio_setup();
    systick_setup();
    i2c_setup();

    while (1) {
        //do nothing
    }
}

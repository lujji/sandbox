#include "i2c.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

void i2c_setup() {
    /* Enable clocks for I2C2 and AFIO. */
    rcc_periph_clock_enable(RCC_I2C2);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_GPIOB);

    /* Set alternate functions for the SCL and SDA pins of I2C2. */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
            GPIO_I2C2_SCL | GPIO_I2C2_SDA);

    /* Disable the I2C before changing any configuration. */
    i2c_peripheral_disable(I2C_BUS);

    /* APB1 is running at 36MHz. */
    i2c_set_clock_frequency(I2C_BUS, I2C_CR2_FREQ_36MHZ);

    /* 400KHz - I2C Fast Mode */
    i2c_set_fast_mode(I2C_BUS);

    /*
     * fclock for I2C is 36MHz APB2 -> cycle time 28ns, low time at 400kHz
     * incl trise -> Thigh = 1600ns; CCR = tlow/tcycle = 0x1C,9;
     * Datasheet suggests 0x1e.
     */
    i2c_set_ccr(I2C_BUS, 0x1e);

    /*
     * fclock for I2C is 36MHz -> cycle time 28ns, rise time for
     * 400kHz => 300ns and 100kHz => 1000ns; 300ns/28ns = 10;
     * Incremented by 1 -> 11.
     */
    i2c_set_trise(I2C_BUS, 0x0b);

    /*
     * This is our slave address - needed only if we want to receive from
     * other masters.
     */
    i2c_set_own_7bit_slave_address(I2C_BUS, 0x32);

    /* If everything is configured -> enable the peripheral. */
    i2c_peripheral_enable(I2C_BUS);
}

void i2c_start() {
    i2c_send_start(I2C_BUS);
    /* Waiting for START is send and switched to master mode. */
    while (!((I2C_SR1(I2C_BUS) & I2C_SR1_SB) & (I2C_SR2(I2C_BUS) & (I2C_SR2_MSL | I2C_SR2_BUSY))));
}

void i2c_stop() {
    i2c_send_stop(I2C_BUS);
    while (!(I2C_SR2(I2C_BUS) & I2C_SR2_BUSY));
}

void i2c_write_addr(uint8_t addr, int rw) {
    /* Say to what address we want to talk to. */
    i2c_send_7bit_address(I2C_BUS, addr, rw);
    /* Waiting for address is transferred. */
    while (!(I2C_SR1(I2C_BUS) & I2C_SR1_ADDR));
    /* Cleaning ADDR condition sequence. */
    (void) I2C_SR2(I2C_BUS);
}

/*
 * @Deprecated
 */
// static uint8_t __i2c_read(int ack) {
//     if (ack) i2c_enable_ack(I2C_BUS);
//     else i2c_disable_ack(I2C_BUS);
//     while (!(I2C_SR1(I2C_BUS) & I2C_SR1_BTF));
//     return i2c_get_data(I2C_BUS);
// }

uint8_t i2c_read() {
    i2c_disable_ack(I2C_BUS);
    /* Cleaning ADDR condition sequence. */
    (void) I2C_SR2(I2C_BUS);
    i2c_stop();
    while (!(I2C_SR1(I2C_BUS) & I2C_SR1_RxNE));
    return i2c_get_data(I2C_BUS);
}

void i2c_read_buf(uint8_t *buf, int len) {
    i2c_enable_ack(I2C_BUS);
    while (len > 2) {
        /* Read into buffer */
        while (!(I2C_SR1(I2C_BUS) & I2C_SR1_RxNE));
        *(buf++) = i2c_get_data(I2C_BUS);
        len--;
    }

    /* Read last 2 bytes */
    while (!(I2C_SR1(I2C_BUS) & I2C_SR1_RxNE)); // should be BTF?
    i2c_disable_ack(I2C_BUS);
    i2c_stop();
    *(buf++) = i2c_get_data(I2C_BUS);


    while (!(I2C_SR1(I2C_BUS) & I2C_SR1_RxNE));
    *buf = i2c_get_data(I2C_BUS);

    i2c_enable_ack(I2C_BUS);
}

void i2c_write(uint8_t data) {
    i2c_send_data(I2C_BUS, data);
    while (!(I2C_SR1(I2C_BUS) & (I2C_SR1_BTF | I2C_SR1_TxE)));
}

#include <stdio.h>
#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/i2c.h>

#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/tpiu.h>
#include <libopencm3/cm3/itm.h>

#define LED_PIN         GPIO13

int _write(int file, char *ptr, int len);

static void clock_setup(void) {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup(void) {
    /* Set GPIO12 on Port C) to 'output push-pull' */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
}

static void delay_ms(int ms) {
    for (int j = 0; j < ms; j++)
        for (int i = 0; i < 4000000; i++)
            __asm__("nop");
}

static void i2c_setup(void) {
    /* Enable clocks for I2C2 and AFIO. */
    rcc_periph_clock_enable(RCC_I2C2);
    rcc_periph_clock_enable(RCC_AFIO);

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

#define DEV_ADDR        (0x1E << 1)
#define DEV_ADDR        0x1E
#define CR_A            0x00
#define CR_B            0x01
#define MODE            0x02
#define DATA_OUT        0x03
#define ID_REG_A        0x0A

static void i2c_start(void) {
    uint32_t i2c = I2C2;
    i2c_send_start(i2c);
    /* Waiting for START is send and switched to master mode. */
    while (!((I2C_SR1(i2c) & I2C_SR1_SB) & (I2C_SR2(i2c) & (I2C_SR2_MSL | I2C_SR2_BUSY))));
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

static void i2c_write(uint8_t data) {
    uint32_t i2c = I2C2;
    i2c_send_data(i2c, data);
    while (!(I2C_SR1(i2c) & (I2C_SR1_BTF | I2C_SR1_TxE)));
}

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

    id[0] = i2c_read(1);
    id[1] = i2c_read(1);

    i2c_disable_ack(i2c);
    id[2] = i2c_get_data(i2c);
    i2c_stop();

    /* 2-byte receive is a special case. See datasheet POS bit. */
    //I2C_CR1(i2c) |= (I2C_CR1_POS | I2C_CR1_ACK);
    /*
     * Yes they mean it: we have to generate the STOP condition before
     * saving the 1st byte.
     */
}

static void uart_setup(void) {
    /* Enable clock for UART peripheral */
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_GPIOA);

    /* Setup GPIO pin GPIO_USART1_TX. */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    /* Setup UART parameters. */
    usart_set_baudrate(USART1, 38400);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    /* Finally enable the USART. */
    usart_enable(USART1);
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

int _write_uart(int file, char *ptr, int len) {
    (void) file;
    int i;
    for (i = 0; i < len; i++)
        usart_send_blocking(USART1, ptr[i]);
    return i;
}

int main(void) {
    clock_setup();
    gpio_setup();
    uart_setup();
    i2c_setup();

    gpio_set(GPIOC, LED_PIN);

    int i = 0;
    while (1) {
        gpio_toggle(GPIOC, LED_PIN);
        //get_id();
        //printf("Sensor ID: %d%d%d\n", id[0], id[1], id[2]);
        printf("[%d]: Hello, world\n", i++);
        printf("[%d]: Hello, world\n", i++);
        //delay_ms(1);
    }
}

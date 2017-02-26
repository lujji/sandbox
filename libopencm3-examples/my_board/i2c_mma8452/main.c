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

#include "util.h"
#include "lib/i2c.h"
#include "lib/MMA8452.h"

#define LED_PIN         GPIO13

static void clock_setup() {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    /* Enable GPIOC clock. */
    rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup() {
    /* Set GPIO6/7/8/9 (in GPIO port C) to 'output push-pull'. */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
}

static void systick_setup() {
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
    /* SysTick interrupt every N clock pulses: set reload to N-1 */
    systick_set_reload(4000000);
    systick_interrupt_enable();
    systick_counter_enable();
}


void sys_tick_handler() {
    gpio_toggle(GPIOC, LED_PIN);
    printf(".\n");
}

/*********************************************************/
void MMA8452_write_reg(uint8_t reg, uint8_t val) {
    i2c_start();
    i2c_write_addr(MMA8452_ADDR, I2C_WRITE);
    i2c_write(reg);
    i2c_write(val);
    i2c_stop();
}

uint8_t MMA8452_read_reg(uint8_t reg) {
    i2c_start();
    i2c_write_addr(MMA8452_ADDR, I2C_WRITE);
    i2c_write(reg);

    /* Generate repeated start and read one byte */
    i2c_start();
    i2c_write_addr(MMA8452_ADDR, I2C_READ);
    return i2c_read();
}

void MMA8452_read_regs(uint8_t *dest, uint8_t reg, uint8_t count) {
    i2c_start();
    i2c_write_addr(MMA8452_ADDR, I2C_WRITE);
    i2c_write(reg);

    /* Generate repeated start*/
    i2c_start();
    i2c_write_addr(MMA8452_ADDR, I2C_READ);

    /* Read multiple registers */
    i2c_read_buf(dest, count);
}
/*********************************************************/

void MMA8452_init() {
    MMA8452_write_reg(MMA8452_CTRL_REG1, 0x01);
}

void MMA8452_readXYZ() {
    uint8_t buf[6];
    MMA8452_read_regs(buf, MMA8452_OUT_X_MSB, 6);
}

int main() {
    clock_setup();
    gpio_setup();
    systick_setup();
    i2c_setup();
    MMA8452_init();

    while (1) {
        MMA8452_readXYZ();
        delay_ms(10);
    }
}

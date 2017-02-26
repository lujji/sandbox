#include <stdio.h>
#include <string.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include "trace.h"

#define LED_PIN         GPIO13 // PC13

static void output_set(void);
static void output_clear(void);

uint32_t temp32;
uint32_t temp4s;
uint16_t adc_Eavg = 0;

static void delay_ms(int ms) {
    for (int j = 0; j < ms; j++)
        for (int i = 0; i < 36000; i++) /* Wait a bit. */
            __asm__("nop");
}

int q = 0;
int trigger = 0;
volatile int armed = 0;

void sys_tick_handler() {
    temp32++;
    if (trigger) {
        if (temp32 >= 200) {
            if (!armed)
                gpio_toggle(GPIOC, LED_PIN);
            temp32 = 0;
        }

        if (temp4s++ >= 4000) {
            if (!armed) {
                output_set();
                gpio_clear(GPIOC, LED_PIN);
                temp4s = 0;
                armed = 1;
            }
        }
    } else {
        if (temp32 >= 1500) {
            /* We call this handler every 1ms so 1000ms = 1s on/off. */
            gpio_toggle(GPIOC, LED_PIN);
            temp32 = 0;
        }
    }
}

static void systick_config() {
    /* 72MHz / 8 => 9000000 counts per second */
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    /* 9000000/9000 = 1000 overflows per second - every 1ms one interrupt */
    /* SysTick interrupt every N clock pulses: set reload to N-1 */
    systick_set_reload(8999);

    systick_interrupt_enable();

    /* Start counting. */
    systick_counter_enable();
}

static void clock_config() {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
}

static void gpio_config() {
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);

    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_PUSHPULL, GPIO1 | GPIO2);
    output_clear();
}

static void output_set() {
    gpio_set(GPIOA, GPIO2);
    gpio_clear(GPIOA, GPIO1);
}

static void output_clear() {
    gpio_set(GPIOA, GPIO1);
    gpio_clear(GPIOA, GPIO2);
}

static void adc_config(void) {
    rcc_periph_clock_enable(RCC_ADC1);
    //rcc_periph_clock_enable(RCC_GPIOA);
    //gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);

    /* Make sure the ADC doesn't run during config. */
    adc_off(ADC1);

    /* We configure everything for one single conversion. */
    adc_disable_scan_mode(ADC1);
    adc_set_single_conversion_mode(ADC1);
    adc_disable_external_trigger_regular(ADC1);
    adc_set_right_aligned(ADC1);
    //adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_28DOT5CYC);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_239DOT5CYC);

    adc_power_on(ADC1);

    /* Wait for ADC starting up. */
    int i;
    for (i = 0; i < 800000; i++) /* Wait a bit. */
        __asm__("nop");

    adc_reset_calibration(ADC1);
    adc_calibration(ADC1);

}

static uint16_t adc_read(uint8_t channel) {
    uint8_t channel_array[16];
    channel_array[0] = channel;
    adc_set_regular_sequence(ADC1, 1, channel_array);
    adc_start_conversion_direct(ADC1);
    while (!adc_eoc(ADC1));
    uint16_t reg16 = adc_read_regular(ADC1);
    return reg16;
}

// SysTick +
// ADC
// 2 GPIOS +

void main() {
    clock_config();
    gpio_config();
    systick_config();
    adc_config();

    while (1) {
        uint16_t adc = adc_read(0);
        adc_Eavg = (adc >> 2) + adc_Eavg - (adc_Eavg >> 2);
        if (adc_Eavg < 3000) {
            trigger = 1;
        } else {
            trigger = 0;
            temp4s = 0;
            armed = 0;
            output_clear();
        }
        //printf("adc: %d\n", adc_Eavg);
    }
}

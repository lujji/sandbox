#include <stdio.h>
#include <string.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define LED_PIN         GPIO13 // PC13

static void delay_ms(int ms) {
    while(ms--) {
        for (uint32_t i = 0; i < (24000000UL / 4000) * 2; i++) {
            __asm__("nop");
        }
    }
}

void delay_us(int us) {
    for (int i = 0; i < 7 * us; i++)
        __asm__("nop");
}

void pin_write(uint16_t gpio, int value) {
    if (value)
        gpio_set(GPIOA, gpio);
    else
        gpio_clear(GPIOA, gpio);
}


#define HD44780_RS          GPIO1
#define HD44780_EN          GPIO0

void LCD_write4(uint8_t cmd) {
    gpio_set(GPIOA, HD44780_EN);
    pin_write(GPIO2, cmd & (1 << 3));
    pin_write(GPIO3, cmd & (1 << 2));
    pin_write(GPIO4, cmd & (1 << 1));
    pin_write(GPIO5, cmd & (1 << 0));
    gpio_clear(GPIOA, HD44780_EN);

    delay_ms(10);
}

/*
 * E  -> A0
 * RS -> A1
 * RW -> GND
 * D4 -> A5
 * D5 -> A4
 * D6 -> A3
 * D7 -> A2
 */
void LCD_write(uint8_t cmd) {
    LCD_write4(cmd >> 4);
    LCD_write4(cmd & 0x0f);
}

void LCD_putc(char c) {
    gpio_set(GPIOA, HD44780_RS);
    LCD_write(c);
}

void LCD_goto(uint8_t col, uint8_t row) {
    gpio_clear(GPIOA, HD44780_RS);
    if (row == 0)
        LCD_write(0x80 + col);
    else
        LCD_write(0xC0 + col);
}

void LCD_init(void) {
    gpio_clear(GPIOA, GPIO2 | GPIO3 | GPIO4 | GPIO5 | HD44780_RS | HD44780_EN);

    delay_ms(100);

    /* reset sequence */
    for (int i = 0; i < 3; i++) {
        LCD_write4(0x03);
        delay_ms(5);
    }

    /* LCD initialization */
    LCD_write4(0x02); // 4-bit mode
    delay_ms(20);
    LCD_write(0x28); // 2-line mode
    LCD_write(0x08); // off
    LCD_write(0x01); // clear
    LCD_write(0x06); // entry mode
    LCD_write(0x0C); // on
}

void gpio_init(void) {
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO5 | GPIO4 | GPIO3 | GPIO2 | GPIO1 | GPIO0);
}

void main() {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_init();

    LCD_init();

    int i = 'A';
    LCD_goto(0, 1);
    char txt[] = "Hello, World!";
    for (int i=0; i<sizeof(txt) - 1; i++)
        LCD_putc(txt[i]);
    while (1) {
        gpio_toggle(GPIOC, LED_PIN);
        delay_ms(20);
        //LCD_putc(i++);
        //LCD_goto(10, 1);
    }
}

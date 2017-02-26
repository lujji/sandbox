#ifndef LCD_PROTOCOL_H_
#define LCD_PROTOCOL_H_

#include <stdint.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

/* LCD connection:
 *
 * 1 RST -> PA1
 * 2 CE  -> PA2
 * 3 DC  -> PA3
 * 4 DIN -> PA7 (MOSI)
 * 5 CLK -> PA5 (SCK)
 * 6 VCC
 * 7 LED
 * 8 GND
 */

/* Pin configuration */
#define DC_PIN  GPIO3
#define SCE_PIN GPIO2
#define RST_PIN GPIO1


/**
 * set LCD in Data mode
 */
inline void LCD_DC_set()
{
    gpio_set(GPIOA, DC_PIN);
}

/**
 * set LCD in command mode
 */
inline void LCD_DC_clear()
{
    gpio_clear(GPIOA, DC_PIN);
}

/**
 * Chip select
 */
inline void LCD_SCE_set()
{
    gpio_set(GPIOA, SCE_PIN);
}

inline void LCD_SCE_clear()
{
    gpio_clear(GPIOA, SCE_PIN);
}

inline void LCD_RST_set()
{
    gpio_set(GPIOA, RST_PIN);
}

inline void LCD_RST_clear()
{
    gpio_clear(GPIOA, RST_PIN);
}

/* Platform-specific */
inline void LCD_SPI_write(uint8_t word)
{
    spi_xfer(SPI1, (uint8_t) word);
    //uint8_t reg = spi_xfer(SPI1, (uint8_t) 0xff);
}

inline void LCD_delay_ms(int ms)
{
    for (int i = 0; i < 4000000; i++)
        __asm__("nop");
}

/* LCD functions */
void LCD_initialize();
void LCD_clear();
void LCD_step();
void LCD_sendCommand(unsigned char command);
void LCD_goto(unsigned char x, unsigned char y);
void LCD_writeByte(unsigned char data);
void LCD_writeBlock(unsigned char* block, unsigned int size);
void LCD_writeChar(unsigned char ch);
void LCD_inverse(uint8_t state);
void saveLCDPosition();
void restoreLCDPosition();

#endif /* LCD_PROTOCOL_H_ */

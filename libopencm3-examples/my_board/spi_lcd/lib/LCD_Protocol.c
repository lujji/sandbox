#include "LCD_Protocol.h"

static const unsigned char font[] = {
    0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b10000000, //SPACE
    0b01011100, 0b10000000, 0b10000000, 0b10000000, 0b10000000, //!
    0b00001100, 0b00000000, 0b00001100, 0b10000000, 0b10000000, //"
    0b00101000, 0b01111100, 0b00101000, 0b01111100, 0b00101000, //#
    0b01010000, 0b01011000, 0b11101100, 0b00101000, 0b10000000, //$
    0b00000100, 0b01100000, 0b00010000, 0b00001100, 0b01000000, //%
    0b00101000, 0b01010100, 0b01010100, 0b00100000, 0b01010000, //&
    0b00001100, 0b10000000, 0b10000000, 0b10000000, 0b10000000, //'
    0b00111000, 0b01000100, 0b10000000, 0b10000000, 0b10000000, //(
    0b01000100, 0b00111000, 0b10000000, 0b10000000, 0b10000000, //)
    0b00010100, 0b00001000, 0b00010100, 0b10000000, 0b10000000, //*
    0b00010000, 0b00111000, 0b00010000, 0b10000000, 0b10000000, //+
    0b11000000, 0b01000000, 0b10000000, 0b10000000, 0b10000000, //,
    0b00010000, 0b00010000, 0b00010000, 0b10000000, 0b10000000, //-
    0b01000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, //.
    0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, ///
    0b00111000, 0b01000100, 0b01000100, 0b00111000, 0b10000000, //0
    0b00000100, 0b01111100, 0b10000000, 0b10000000, 0b10000000, //1
    0b01100100, 0b01010100, 0b01010100, 0b01001000, 0b10000000, //2
    0b01000100, 0b01010100, 0b01010100, 0b00101000, 0b10000000, //3
    0b00110000, 0b00101000, 0b01111100, 0b00100000, 0b10000000, //4
    0b01011100, 0b01010100, 0b01010100, 0b00100100, 0b10000000, //5
    0b00111000, 0b01010100, 0b01010100, 0b00100000, 0b10000000, //6
    0b00000100, 0b01100100, 0b00010100, 0b00001100, 0b10000000, //7
    0b00101000, 0b01010100, 0b01010100, 0b00101000, 0b10000000, //8
    0b00001000, 0b01010100, 0b01010100, 0b00111000, 0b10000000, //9
    0b00101000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, //:
    0b01101000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, //;
    0b00010000, 0b00101000, 0b01000100, 0b10000000, 0b10000000, //<
    0b00101000, 0b00101000, 0b00101000, 0b10000000, 0b10000000, //=
    0b01000100, 0b00101000, 0b00010000, 0b10000000, 0b10000000, //>
    0b00000100, 0b01010100, 0b00010100, 0b00001000, 0b10000000, //?
    0b00111000, 0b01000100, 0b01110100, 0b01010100, 0b00111000, //@
    0b01111000, 0b00100100, 0b00100100, 0b01111000, 0b10000000, //A
    0b01111100, 0b01010100, 0b01010100, 0b00101000, 0b10000000, //B
    0b00111000, 0b01000100, 0b01000100, 0b10000000, 0b10000000, //C
    0b01111100, 0b01000100, 0b01000100, 0b00111000, 0b10000000, //D
    0b01111100, 0b01010100, 0b01010100, 0b01000100, 0b10000000, //E
    0b01111100, 0b00010100, 0b00010100, 0b00000100, 0b10000000, //F
    0b00111000, 0b01000100, 0b01010100, 0b01110100, 0b10000000, //G
    0b01111100, 0b00010000, 0b00010000, 0b01111100, 0b10000000, //H
    0b01000100, 0b01111100, 0b01000100, 0b10000000, 0b10000000, //I
    0b00100000, 0b01000000, 0b01000100, 0b00111100, 0b10000000, //J
    0b01111100, 0b00010000, 0b00101000, 0b01000100, 0b10000000, //K
    0b01111100, 0b01000000, 0b01000000, 0b10000000, 0b10000000, //L
    0b01111100, 0b00001000, 0b00010000, 0b00001000, 0b01111100, //M
    0b01111100, 0b00001000, 0b00010000, 0b01111100, 0b10000000, //N
    0b00111000, 0b01000100, 0b01000100, 0b00111000, 0b10000000, //O
    0b01111100, 0b00100100, 0b00100100, 0b00011000, 0b10000000, //P
    0b00111000, 0b01000100, 0b01000100, 0b10111000, 0b10000000, //Q
    0b01111100, 0b00100100, 0b00100100, 0b01011000, 0b10000000, //R
    0b01001000, 0b01010100, 0b01010100, 0b00100100, 0b10000000, //S
    0b00000100, 0b01111100, 0b00000100, 0b10000000, 0b10000000, //T
    0b00111100, 0b01000000, 0b01000000, 0b00111100, 0b10000000, //U
    0b00111100, 0b01000000, 0b00110000, 0b00001100, 0b10000000, //V
    0b00111100, 0b01000000, 0b00111000, 0b01000000, 0b00111100, //W
    0b01101100, 0b00010000, 0b00010000, 0b01101100, 0b10000000, //X
    0b00001100, 0b01010000, 0b01010000, 0b00111100, 0b10000000, //Y
    0b01100100, 0b01010100, 0b01001100, 0b10000000, 0b10000000, //Z
    0b01111100, 0b01000100, 0b10000000, 0b10000000, 0b10000000, // [
    0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, /* \ */
    0b01000100, 0b01111100, 0b10000000, 0b10000000, 0b10000000, // ]
    0b00001000, 0b00000100, 0b00001000, 0b10000000, 0b10000000, // ^
    0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b10000000, // _
    0b00000100, 0b00001000, 0b10000000, 0b10000000, 0b10000000, // `
    0b00110000, 0b01001000, 0b01001000, 0b01111000, 0b10000000, // a
    0b01111100, 0b01001000, 0b01001000, 0b00110000, 0b10000000, // b
    0b00110000, 0b01001000, 0b01001000, 0b10000000, 0b10000000, // c
    0b00110000, 0b01001000, 0b01001000, 0b01111100, 0b10000000, // d
    0b00110000, 0b01101000, 0b01011000, 0b00010000, 0b10000000, // e
    0b00010000, 0b01111000, 0b00010100, 0b10000000, 0b10000000, // f
    0b00011000, 0b10100100, 0b10100100, 0b01111100, 0b10000000, // g
    0b01111100, 0b00001000, 0b00001000, 0b01110000, 0b10000000, // h
    0b01110100, 0b10000000, 0b10000000, 0b10000000, 0b10000000, // i
    0b01000000, 0b00110100, 0b10000000, 0b10000000, 0b10000000, // j
    0b01111100, 0b00100000, 0b00110000, 0b01000100, 0b10000000, // k
    0b01111100, 0b10000000, 0b10000000, 0b10000000, 0b10000000, // l
    0b01111000, 0b00001000, 0b01111000, 0b00001000, 0b01110000, // m
    0b01111000, 0b00001000, 0b00001000, 0b01110000, 0b10000000, // n
    0b00110000, 0b01001000, 0b01001000, 0b00110000, 0b10000000, // o
    0b11111000, 0b01001000, 0b01001000, 0b00110000, 0b10000000, // p
    0b00110000, 0b01001000, 0b01001000, 0b11111000, 0b10000000, // q
    0b01111000, 0b00010000, 0b00001000, 0b10000000, 0b10000000, // r
    0b01010000, 0b01011000, 0b01101000, 0b00101000, 0b10000000, // s
    0b00001000, 0b00111100, 0b01001000, 0b10000000, 0b10000000, // t
    0b00111000, 0b01000000, 0b01000000, 0b01111000, 0b10000000, // u
    0b00111000, 0b01000000, 0b00100000, 0b00011000, 0b10000000, // v
    0b00011000, 0b01100000, 0b00011000, 0b01100000, 0b00011000, // w
    0b01001000, 0b00110000, 0b01001000, 0b10000000, 0b10000000, // x
    0b00011000, 0b10100000, 0b10100000, 0b01111000, 0b10000000, // y
    0b01001000, 0b01101000, 0b01011000, 0b01001000, 0b10000000, // z
    0b00010000, 0b01101100, 0b01000100, 0b10000000, 0b10000000, // {
    0b01111110, 0b10000000, 0b10000000, 0b10000000, 0b10000000, // |
    0b01000100, 0b01101100, 0b00010000, 0b10000000, 0b10000000, // }
    0b00001000, 0b00000100, 0b00001000, 0b00000100, 0b10000000 // ~
};

void LCD_initialize(void)
{
    //LCD_delay_ms(50); // Allow some time to let things settle
    LCD_SCE_clear(); // Enable LCD writing
    LCD_RST_clear(); // Reset LCD
    LCD_delay_ms(50); // Allow the LCD controller some time to reset
    LCD_RST_set(); // Disable reset
    LCD_SCE_set(); // Disable LCD writing
    LCD_sendCommand(0x21); // LCD Extended Commands.
    //LCD_sendCommand(0xD0); // Set LCD Vop (Contrast).
    //LCD_sendCommand(0xb8); // 0xB2 -> 0xBC 0xaf
    LCD_sendCommand(0xbc); // 0xB2 -> 0xBC 0xaf

    LCD_sendCommand(0x04); // Set Temp coefficent.
    LCD_sendCommand(0x13); // LCD bias mode 1:48.
    LCD_sendCommand(0x20); // LCD Standard Commands, Horizontal addressing mode.
    LCD_sendCommand(0x0C); // LCD in normal mode. 0x0d??

    //LCD_sendCommand(0b10101000); // LCD in normal mode.
    //LCD_sendCommand(0b00110000); // LCD in normal mode. = 0x30

    LCD_clear(); // Clear the LCD so it's ready for use
}

void LCD_inverse(uint8_t state)
{
    if (state == 1)
        LCD_sendCommand(0x0D);
    else
        LCD_sendCommand(0x0C);
}

void LCD_sendCommand(unsigned char command)
{
    LCD_SCE_clear();
    LCD_DC_clear();
    LCD_SPI_write(command);
    LCD_SCE_set();
}

void LCD_writeByte(unsigned char data)
{
    LCD_SCE_clear();
    LCD_DC_set();
    LCD_SPI_write(data);
    LCD_SCE_set();
}

void LCD_writeBlock(unsigned char* block, unsigned int size)
{
    uint8_t *p = block;
    while (p < block + size)
        LCD_writeByte(*(p++));
}

void LCD_clear(void)
{
    LCD_goto(0, 0);
    uint16_t i = 504;
    while (i > 0) {
        LCD_writeByte(0);
        i--;
    }
}

/*
 * x range: 0 to 83
 * y range: 0 to 5
 */
void LCD_goto(unsigned char x, unsigned char y)
{
    LCD_sendCommand(0x80 | x);
    LCD_sendCommand(0x40 | y);
}

void LCD_writeChar(unsigned char ch)
{
    for (uint8_t i = 0; i < 5; i++) {
        if (font [(ch - 32)*5 + i] == 0b10000000)
            continue;
        LCD_writeByte(font [(ch - 32)*5 + i]);
    }
    LCD_writeByte(0x00);
}

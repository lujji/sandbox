#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <libopencm3/stm32/i2c.h>

#define I2C_BUS	I2C2

/**
 * SDA -> PB11
 * SCL -> PB10
 */
void i2c_setup();

/**
 * Generate START condition.
 */
void i2c_start();

/**
 * Generate STOP condition.
 */
void i2c_stop();

/**
 * Generate START condition and write slave address.
 *
 * @param addr slave address (not shifted)
 * @param rw I2C_READ or I2C_WRITE
 */
void i2c_write_addr(uint8_t addr, int rw);

/*
 * Read one byte. STOP is generated automatically.
 */
uint8_t i2c_read();

/**
 * Read >= 2 bytes. STOP is generated automatically.
 *
 * @param buf destination buffer
 * @param len buffer length
 */
void i2c_read_buf(uint8_t *buf, int len);

/**
 * Write one byte.
 *
 * @param data
 */
void i2c_write(uint8_t data);

#endif /* I2C_H */

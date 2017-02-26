#include "MMA8452.h"
#include "i2c.h"

uint8_t MMA8452_getID(void) {
    i2c_start();
    i2c_write_addr(MMA8452_ADDR, I2C_WRITE);
    i2c_write(MMA8452_WHO_AM_I);

    /* Generate repeated start and read one byte*/
    i2c_start();
    i2c_write_addr(MMA8452_ADDR, I2C_READ);
    return i2c_read();
}


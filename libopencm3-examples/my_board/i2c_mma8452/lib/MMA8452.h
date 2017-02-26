#ifndef MMA8452_H
#define MMA8452_H

#include <stdint.h>

#define MMA8452_ADDR        0x1C
#define MMA8452_DEVICE_ID   0x2A

#define MMA8452_STATUS      0x00
#define MMA8452_WHO_AM_I    0x0D
#define MMA8452_OUT_X_MSB   0x01
#define MMA8452_OUT_X_LSB   0x02
#define MMA8452_OUT_Y_MSB   0x03
#define MMA8452_OUT_Y_LSB   0x04
#define MMA8452_OUT_Z_MSB   0x05
#define MMA8452_OUT_Z_LSB   0x06
#define MMA8452_CTRL_REG1   0x2A
#define MMA8452_CTRL_REG2   0x2B
#define MMA8452_CTRL_REG3   0x2C
#define MMA8452_CTRL_REG4   0x2D
#define MMA8452_CTRL_REG5   0x2E

uint8_t MMA8452_getID(void);

#endif /* MMA8452_H */
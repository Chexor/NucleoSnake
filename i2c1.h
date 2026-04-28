#ifndef I2C1_H
#define I2C1_H

#include "stm32f091xc.h"
#include <stdint.h>

void I2C1_Init(void);
void I2C1_WriteArray(uint8_t deviceAddress, const uint8_t* data, uint8_t length);
void I2C1_WriteRegisterByte(uint8_t deviceAddress, uint8_t reg, uint8_t value);
uint8_t I2C1_ReadRegisterByte(uint8_t deviceAddress, uint8_t reg);

#endif // I2C1_H

#include "stm32f091xc.h"

#if !defined(I2C1_DEFINED)
	#define I2C1_DEFINED
	
	void InitI2C1(void);
	void I2C1WriteRegisterByte(uint8_t deviceAddress, uint8_t registerToWrite, uint8_t dataToWrite);
	void I2C1WriteRegisterArray(uint8_t deviceAddress, uint8_t registerToWrite, uint8_t* data, uint8_t length);
	void I2C1WriteArray(uint8_t deviceAddress, uint8_t* data, uint8_t length);
	uint8_t I2C1ReadRegisterByte(uint8_t deviceAddress, uint8_t registerToRead);
#endif

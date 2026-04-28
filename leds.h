#include "stm32f091xc.h"
#include "stdbool.h"

#if !defined(LEDS_DEFINED)
	#define LEDS_DEFINED
	
	void InitLeds(void);
	void SetLed(uint8_t ledNumber);
	void ClearLed(uint8_t ledNumber);
	void ToggleLed(uint8_t ledNumber);
	void FlashLed(uint8_t ledNumber, uint8_t flashTime);
	void ByteToLeds(uint8_t data);
	void ByteToLevel(uint8_t level);
#endif

#include "stm32f091xc.h"
#include "stdbool.h"

#if !defined(MAIN_DEFINED)
	#define MAIN_DEFINED
	
	// Functie prototypes.
	void SystemClock_Config(void);
	void WaitForMs(uint32_t timespan);
#endif

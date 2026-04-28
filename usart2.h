#include "stm32f091xc.h"

#if !defined(USART2_DEFINED)
	#define USART2_DEFINED
	
	void InitUsart2(uint32_t baudRate);	
	void StringToUsart2(char* string);
#endif


/*

Usage in main.c

void USART2_IRQHandler(void)
{
	// Is er een byte ontvangen?
	if((USART2->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
	{
		// Byte ontvangen, lees hem om alle vlaggen te wissen.
		uint8_t temp = USART2->RDR;		
		
		...
	}
	
	// Timeout op de ontvangst... Gebruik die om te synchroniseren op de seriële communicatie.
	// Dit concept zorgt er bijvoorbeeld voor dat wanneer er geen '\n' ontvangen wordt, 
	// toch opnieuw goed gesynchroniseerd wordt.
	if((USART2->ISR & USART_ISR_RTOF) == USART_ISR_RTOF)
	{
		// Vlag wissen.
		USART2->ICR |= USART_ICR_RTOCF;
		
		// Data-ontvangst hersynchroniseren.
		indexer = 0;
	}
}

*/

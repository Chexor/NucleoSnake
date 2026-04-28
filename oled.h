#include "stm32f091xc.h"
#include "stdbool.h"

#if !defined(OLED_DEFINED)
	#define OLED_DEFINED
	
	// Functies om een 0.96 inch OLED display met SSD1306 IC aan te sturen via I²C.
	#define OLED_SSD1306_ADDRESS 			0x3C					// 7-bit address. Moet nog één plaats naar links opgeschoven worden.
	#define OLED_COMMAND_CONTROL_BYTE	0x00					// Contiuation bit + Command bit + 6 control bits.
	#define OLED_DATA_CONTROL_BYTE 		0x40					// Contiuation bit + Data bit + 6 control bits.	

	#define OLED_WIDTH								128						// Scherm is 128 pixels breed.
	#define OLED_HEIGHT								64						// Scherm is 64 pixels hoog.
	#define OLED_NUMBER_OF_PAGES			8							// Indien je een font gebruikt van 8 pixels hoog, heb je dan 8 lijnen tekst mogelijk.
	
	void OLED_Init(void);
	void OLED_SetColumnStartAddress(uint8_t startAddress);
	void OLED_SetPageStartAddress(uint8_t startAddress);
	void OLED_FillScreen(uint8_t pageColumnData);
	void OLED_FillPage(uint8_t pageColumnData, uint8_t pageNumber);
	void OLED_StringToPage(char* text, uint8_t page, bool fillWithBlanks);
	void OLED_ArrayToPage(uint8_t* data, uint8_t dataLength, uint8_t pageNumber);
	void OLED_ImageToScreen(uint8_t image[][OLED_WIDTH]);
#endif

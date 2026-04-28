#include "stm32f091xc.h"
#include "stdbool.h"
#include "oled.h"
#include "i2c1.h"
#include "string.h"
#include "font6x8.h"

void OLED_Init(void)
{	
	// Het OLED-display initialiseren. 
	// De opstartsequentie is terug te vinden op het einde van de 
	// datasheet van het SSD1306 IC...
	
	uint8_t i2cData[2];
	i2cData[0] = OLED_COMMAND_CONTROL_BYTE;
	
	// Display uitschakelen?
	i2cData[1] = 0xAE;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set MUX.
	i2cData[1] = 0xA8;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	i2cData[1] = 0x3F;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set display offset.
	i2cData[1] = 0xD3;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	i2cData[1] = 0x00;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set display start line.
	i2cData[1] = 0x40;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set memory addressing mode (page addressing mode).
	i2cData[1] = 0x20;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	i2cData[1] = 0x02;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set segment re-map (screen upside down).
	i2cData[1] = 0xA1;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set COM output scan direction (mirror screen left/right).
	i2cData[1] = 0xC8;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set COM pins hardware configuration (alternative COM pin configuration).
	i2cData[1] = 0xDA;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	i2cData[1] = 0x12;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set contrast control.
	i2cData[1] = 0x81;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	i2cData[1] = 0x7F;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Entire display on.
	i2cData[1] = 0xA4;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set normal display.
	i2cData[1] = 0xA6;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	// OR
	//	// Set inverted display.
	//	i2cData[1] = 0xA7;
	//	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set oscillator frequency.
	i2cData[1] = 0xD5;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	i2cData[1] = 0x80;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Enable charge pump.
	i2cData[1] = 0x8D;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	i2cData[1] = 0x14;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Display on.
	i2cData[1] = 0xAF;
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Fill screen with blanks...
	OLED_FillScreen(0);;
}

// Opgeven in welke kolom het schrijven moet starten.
void OLED_SetColumnStartAddress(uint8_t startAddress)
{
	uint8_t i2cData[2];
	i2cData[0] = OLED_COMMAND_CONTROL_BYTE;
	
	// Set lower column start address (for page addressing mode).
	i2cData[1] = (startAddress & 0x0F);
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
	
	// Set higher column start address (for page addressing mode).
	i2cData[1] = (0x10 | ((startAddress & 0xF0) >> 4));
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
}

// Opgeven op welke page het schrijven moet starten.
void OLED_SetPageStartAddress(uint8_t startAddress)
{	
	uint8_t i2cData[2];
	i2cData[0] = OLED_COMMAND_CONTROL_BYTE;
	
	// Set page start address (for page addressing mode).
	// OPM: indien een te hoog adres opgegeven wordt, zal het 0x07 masker 
	// beletten dat er op die 'hogere' adressen geschreven wordt.
	i2cData[1] = (0xB0 | (startAddress & 0x07));
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, 2);
}

// Volledige page vullen met één bepaalde kolominhoud.
void OLED_FillPage(uint8_t pageColumnData, uint8_t pageNumber)
{
	uint8_t index = 0;
	uint8_t data[OLED_WIDTH + 1];
	
	// Data klaarzetten.
	data[0] = OLED_DATA_CONTROL_BYTE;
	for(index = 1; index < OLED_WIDTH + 1; index++)
		data[index] = pageColumnData;
	
	// Verwijzen naar de juiste page.
	OLED_SetPageStartAddress(pageNumber);		
	
	// Naar het begin van de page gaan.
	OLED_SetColumnStartAddress(0);
	
	// Data verzenden.
	I2C1WriteArray(OLED_SSD1306_ADDRESS, data, OLED_WIDTH + 1);
}

// Volledig scherm vullen met één bepaalde kolominhoud.
void OLED_FillScreen(uint8_t pageColumnData)
{
	uint8_t count = 0;
	
	for(count = 0; count < OLED_NUMBER_OF_PAGES; count++)
	{
		OLED_FillPage(pageColumnData, count);
	}
}

// String schrijven naar één bepaalde pagina. In totaal zijn er acht pagina's (0 -> 7).
// Indien 6x8 karakters gebruikt worden, kunnen er maximaal 21 karakters 
// op één lijn (21 x 6 = 126 < 128).
void OLED_StringToPage(char* text, uint8_t pageNumber, bool fillWithBlanks)
{
	uint16_t fontStartIndexer = 0, textIndexer = 0;
	uint8_t columnCounter = 0;
	uint8_t i2cData[OLED_WIDTH + 1];
	
	// Control byte klaarzetten.
	i2cData[0] = OLED_DATA_CONTROL_BYTE;
	
	// Array opvullen met de bytes uit het font, zolang er tekst is of de breedte van het display bereikt is.
	// Ook controleren op '\r' en '\n', want die karakters staan niet in het font...!
	// TODO: nog betere controle doen op karakters die niet in het font zitten. Want bij gebruik onbestaande karakters, crasht de code mogelijks (out of range).
	for(textIndexer = 0; 
			((textIndexer < strlen(text)) && (((textIndexer + 1) * CHARACTER_WIDTH) < OLED_WIDTH) && (text[textIndexer] != '\r') && (text[textIndexer] != '\n')); 
			textIndexer++)
	{
		// Startpunt in het font array bepalen door het startkarakter ervan af te trekken.	
		fontStartIndexer = (uint8_t)text[textIndexer] - (uint8_t)START_CHARACTER;
		
		// De kolommen opvullen met de font data voor één bepaald karakter.
		for(columnCounter = 0; columnCounter < CHARACTER_WIDTH; columnCounter++)
			// Index + 1 omdat de control byte vóór de echte data zit.
			i2cData[textIndexer * CHARACTER_WIDTH + columnCounter + 1] = font6x8[fontStartIndexer * CHARACTER_WIDTH + columnCounter];
	}
	
	// Indien gewenst, het overblijvende (achterste deel) van de lijn opvullen met spaties.
	if(fillWithBlanks)
	{		
		// Tekst langer maken om de volledige lijn op te vullen (normaal is dat 21 karakters indien je een font van 6 breed gebruikt).
		// OPM: je zou ook eerst de lijn kunnen wissen, maar dat geeft een knipperend effect...
		while(textIndexer < (uint8_t)(OLED_WIDTH/CHARACTER_WIDTH))
		{
			// Startpunt in het font array bepalen door het startkarakter ervan af te trekken.
			// Dat startkarakter is hier een spatie ' '.
			fontStartIndexer = (uint8_t)' ' - (uint8_t)START_CHARACTER;
			
			// De kolommen opvullen met de font data voor één bepaald karakter.
			for(columnCounter = 0; columnCounter < CHARACTER_WIDTH; columnCounter++)
				// Index + 1 omdat de control byte vóór de echte data zit.
				i2cData[textIndexer * CHARACTER_WIDTH + columnCounter + 1] = font6x8[fontStartIndexer * CHARACTER_WIDTH + columnCounter];
			
			// Eén karakter opschuiven.
			textIndexer++;
		}
	}

	// Verwijzen naar de juiste page.
	OLED_SetPageStartAddress(pageNumber);		
	
	// Naar het begin van de page gaan.
	OLED_SetColumnStartAddress(0);
	
	// Data verzenden (met extra control byte).
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, (textIndexer * CHARACTER_WIDTH + 1));
}

// Data-array schrijven naar één bepaalde pagina. Vooral gebruikt om (delen van) afbeeldingen te verzenden.
void OLED_ArrayToPage(uint8_t* data, uint8_t dataLength, uint8_t pageNumber)
{
	uint8_t count = 0;
	uint8_t i2cData[OLED_WIDTH + 1];
	
	// Control byte klaarzetten.
	i2cData[0] = OLED_DATA_CONTROL_BYTE;
	
	// Grenzen bewaken.
	if(dataLength > OLED_WIDTH)
		dataLength = OLED_WIDTH;
	
	// Data kopiëren.
	for(count = 1; count < dataLength + 1; count++)
		i2cData[count] = data[count - 1];
	
	// Verwijzen naar de juiste page.
	OLED_SetPageStartAddress(pageNumber);		
	
	// Naar het begin van de page gaan.
	OLED_SetColumnStartAddress(0);
	
	// Data verzenden.
	I2C1WriteArray(OLED_SSD1306_ADDRESS, i2cData, dataLength + 1);
}

// Schermvullende data schrijven (afbeelding verzenden).
void OLED_ImageToScreen(uint8_t image[][OLED_WIDTH])
{
	uint8_t count = 0;
	
	for(count = 0; count < OLED_NUMBER_OF_PAGES; count++)
		OLED_ArrayToPage((uint8_t *)image[count], OLED_WIDTH, count);		
}

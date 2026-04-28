# NucleoSnake

Snake game voor de STM32F091RC Nucleo met het DM-OLED096-636 / SSD1306 OLED-display op I2C1.

De game toont een klassieke Snake op een 128x64 OLED-scherm. De slang beweegt automatisch vooruit. Met `SW1` draai je links ten opzichte van de huidige richting en met `SW4` draai je rechts. Na een botsing verschijnt een game-over scherm en kan het spel opnieuw gestart worden met de blauwe Nucleo-knop `B1`.

## Hardware

- Board: STM32F091RC Nucleo
- Display: DM-OLED096-636 / SSD1306, 128x64 pixels
- OLED SCL: D15 / PB8
- OLED SDA: D14 / PB9
- SW1 links draaien: PA1
- SW4 rechts draaien: PC1
- Reset na game-over: Nucleo B1 / PC13

Controleer voor het testen dat `JP5` op `U5V` staat, `JP6` geplaatst is, en de `CN2` ST-Link jumpers aanwezig zijn.

## Bediening

- `SW1`: draai 90 graden naar links
- `SW4`: draai 90 graden naar rechts
- `B1`: start opnieuw na game-over

De besturing is relatief. Dat betekent dat `SW1` niet altijd naar links op het scherm beweegt, maar links draait ten opzichte van waar de slang nu naartoe beweegt. Als de slang bijvoorbeeld omhoog beweegt, dan draait `SW1` naar links. Als de slang naar rechts beweegt, dan draait `SW1` naar boven.

## Build

Open `NucleoSnake.uvprojx` in Keil uVision en build target `Nucleo_One`.

Als Keil via de command line beschikbaar is, kan dit ook met:

```bat
UV4.exe -b "labs\NucleoSnake\NucleoSnake.uvprojx" -t "Nucleo_One"
```

## Projectbestanden

- `main.c`: bevat de game loop, Snake-logica, rendering en klokconfiguratie.
- `buttons.c` / `buttons.h`: initialisatie en uitlezen van SW1, SW2, SW3, SW4 en B1.
- `i2c1.c` / `i2c1.h`: I2C1-driver voor communicatie met het OLED-display.
- `oled.c` / `oled.h`: SSD1306 OLED-functies, zoals pagina's vullen en tekst tonen.
- `font6x8.h`: bitmap-font voor tekst op het OLED-display.
- `NucleoSnake.uvprojx`: Keil uVision projectbestand.

## Schermindeling

Het OLED-scherm is 128 pixels breed en 64 pixels hoog. De game gebruikt een vaste pixelborder rond het scherm.

Belangrijke constanten in `main.c`:

- `CELL_SIZE`: elke Snake-cel is 4x4 pixels.
- `FIELD_X_OFFSET`: de horizontale offset binnen de border.
- `FIELD_Y_OFFSET`: de verticale offset binnen de border.
- `FIELD_W`: het speelveld is 31 cellen breed.
- `FIELD_H`: het speelveld is 15 cellen hoog.
- `GAME_STEP_MS`: de slang zet elke 140 ms een stap.

De border staat op de buitenste pixelrand van het OLED-scherm. De cellen beginnen op offset `2,2`, zodat de slang en het voedsel niet over de border getekend worden.

## Game State

De toestand van de game wordt opgeslagen in enkele globale variabelen in `main.c`.

- `snakeX[]`: x-posities van alle slangsegmenten.
- `snakeY[]`: y-posities van alle slangsegmenten.
- `snakeLength`: huidige lengte van de slang.
- `foodX` en `foodY`: positie van het voedsel.
- `direction`: huidige bewegingsrichting.
- `nextDirection`: richting die bij de volgende stap gebruikt wordt.
- `gameOver`: geeft aan of het spel gestopt is door een botsing.
- `randomState`: eenvoudige pseudo-random toestand voor voedselplaatsing.

De kop van de slang zit altijd op index `0`. Segment `1` volgt de kop, segment `2` volgt segment `1`, enzovoort.

## Rendering

De game tekent niet direct pixel per pixel naar het OLED-display. Eerst wordt er getekend in een RAM-buffer:

```c
static uint8_t framebuffer[OLED_NUMBER_OF_PAGES][OLED_WIDTH];
```

Een SSD1306-display is ingedeeld in 8 pages van elk 8 pixels hoog. Daarom bestaat de framebuffer uit 8 rijen van 128 bytes.

De rendering gebeurt in deze volgorde:

1. `ClearFrame()` wist de framebuffer.
2. `DrawBorder()` tekent de rand rond het scherm.
3. `DrawCell(foodX, foodY, false)` tekent het voedsel.
4. De `for`-lus in `RenderGame()` tekent alle slangsegmenten.
5. `SendFrameToOled()` stuurt alle 8 pages naar het OLED-display.

`SetPixel()` vertaalt een x/y-pixelpositie naar de juiste OLED-page en bitpositie:

```c
uint8_t page = y >> 3;
uint8_t bit = y & 0x07U;
framebuffer[page][x] |= (uint8_t)(1U << bit);
```

Hierdoor kan de code eenvoudige pixelcoordinaten gebruiken, terwijl de OLED-driver nog altijd in SSD1306-pages werkt.

## Input

De knopfuncties staan in `buttons.c`.

`InitButtons()` zet de GPIO-clocks aan en configureert de knop-pinnen als input:

- `SW1Active()` leest PA1.
- `SW2Active()` leest PA4.
- `SW3Active()` leest PB0.
- `SW4Active()` leest PC1.
- `UserButtonActive()` leest PC13.

Voor Snake worden alleen `SW1Active()`, `SW4Active()` en `UserButtonActive()` gebruikt.

De functie `ReadInput()` gebruikt flankdetectie. Daardoor draait de slang maar een keer per druk, ook als de knop langer ingedrukt blijft:

```c
if(sw1IsActive && !sw1WasActive)
    nextDirection = TurnLeft(nextDirection);
else if(sw4IsActive && !sw4WasActive)
    nextDirection = TurnRight(nextDirection);
```

Daarna worden `sw1WasActive` en `sw4WasActive` bijgewerkt voor de volgende lus.

## Richtingen

De richting wordt voorgesteld met een enum:

```c
typedef enum
{
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;
```

`TurnLeft()` en `TurnRight()` zetten de huidige richting om naar een nieuwe richting. Voorbeeld voor links draaien:

- `DIR_UP` wordt `DIR_LEFT`
- `DIR_LEFT` wordt `DIR_DOWN`
- `DIR_DOWN` wordt `DIR_RIGHT`
- `DIR_RIGHT` wordt `DIR_UP`

De code gebruikt `nextDirection` in plaats van `direction` direct te wijzigen. Daardoor kan de input tussen twee Snake-stappen al geregistreerd worden, maar wordt de beweging pas toegepast in `MoveSnake()`.

## Beweging

`MoveSnake()` voert een volledige Snake-stap uit.

Eerst wordt de nieuwe koppositie berekend vanuit de huidige richting:

```c
if(direction == DIR_UP)
    newY--;
else if(direction == DIR_DOWN)
    newY++;
else if(direction == DIR_LEFT)
    newX--;
else
    newX++;
```

Daarna controleert de code of de nieuwe positie buiten het speelveld valt. Dat is een botsing met de border en zet `gameOver` op `true`.

Vervolgens wordt gecontroleerd of de kop op het voedsel staat. Als dat zo is, wordt `grow` true en wordt de slang een segment langer.

Daarna controleert de code of de nieuwe koppositie een bestaand slangsegment raakt. Een botsing met de eigen body is game-over. De staartpositie is een uitzondering als de slang niet groeit, want die staart schuift in dezelfde stap weg.

Ten slotte schuiven alle segmenten een plaats op:

```c
for(index = (int16_t)snakeLength - 1; index > 0; index--)
{
    snakeX[index] = snakeX[index - 1];
    snakeY[index] = snakeY[index - 1];
}
```

Daarna krijgt segment `0` de nieuwe koppositie.

## Voedsel

`PlaceFood()` kiest een nieuwe positie met `NextRandom()`.

De random generator is een kleine 16-bit LFSR. Die is eenvoudig, snel en gebruikt geen standaardbibliotheek-randomfunctie. Voor deze game is dat voldoende.

De voedselpositie wordt alleen aanvaard als ze niet op de slang ligt:

```c
if(!SnakeContains(foodX, foodY))
    return;
```

Als er na 512 pogingen geen vrije plaats gevonden wordt, valt de code terug naar positie `0,0`. In normale gameplay gebeurt dat bijna nooit, omdat het speelveld veel groter is dan de startslang.

## Main Loop

`main()` initialiseert eerst alle hardware:

1. `SystemClock_Config()` zet de microcontrollerklok op 48 MHz.
2. `InitButtons()` initialiseert de knoppen.
3. `InitI2C1()` initialiseert I2C1.
4. `OLED_Init()` initialiseert het SSD1306-display.
5. `ResetGame()` zet Snake in de starttoestand.
6. `RenderGame()` tekent het eerste frame.

Daarna blijft de code in een oneindige `while(1)` lus.

Als `gameOver` false is:

- `ReadInput()` leest SW1 en SW4.
- Om de `GAME_STEP_MS` milliseconden voert de game `MoveSnake()` uit.
- Daarna wordt het scherm opnieuw getekend met `RenderGame()`.

Als `gameOver` true is:

- `ShowGameOver()` toont de game-over tekst.
- `UserButtonActive()` controleert of B1 ingedrukt wordt.
- Bij B1 wordt `ResetGame()` uitgevoerd en start de game opnieuw.

## Timing

`SysTick_Handler()` wordt elke 1 ms uitgevoerd en verhoogt `ticks`.

```c
void SysTick_Handler(void)
{
    ticks++;
}
```

De game gebruikt `ticks` voor twee dingen:

- `WaitForMs()` voor korte wachttijden tijdens initialisatie en restart.
- De main loop vergelijkt `ticks - lastStepTick` met `GAME_STEP_MS` om te bepalen wanneer de slang een stap moet zetten.

Omdat `ticks` in een interrupt wordt aangepast, is de variabele `volatile`.

## Game-Over Condities

Het spel eindigt wanneer:

- de kop buiten het speelveld beweegt;
- de kop tegen een eigen slangsegment botst.

De border zelf is visueel getekend op de buitenste pixels. Logisch gezien wordt de border afgedwongen door de grenzen `FIELD_W` en `FIELD_H` in `MoveSnake()`.

## Verwacht Gedrag Op Hardware

Na flashen moet het volgende zichtbaar zijn:

- Er staat een rechthoekige border rond het OLED-scherm.
- De slang start ongeveer in het midden en beweegt naar rechts.
- Voedsel verschijnt als een kleine open cel.
- De slang wordt langer wanneer ze voedsel raakt.
- `SW1` draait de slang links.
- `SW4` draait de slang rechts.
- Botsing met border of eigen lichaam geeft `GAME OVER`.
- `B1` start het spel opnieuw.

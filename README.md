# NucleoSnake

Snake game for the STM32F091RC Nucleo board with an SSD1306 OLED display.

This is a small side project for the IoT1 course at VIVES Kortrijk.

## Features

- Classic Snake gameplay on a 128x64 OLED display.
- Animated start screen with a small snake tongue flick.
- Relative left/right controls using shield buttons.
- Score display on game over.
- Border around the playfield.
- Button debounce and one-turn-per-step input handling.

## Hardware

- Board: STM32F091RC Nucleo
- Display: DM-OLED096-636 / SSD1306, 128x64 pixels
- OLED SCL: D15 / PB8
- OLED SDA: D14 / PB9
- SW1: PA1, turn left
- SW4: PC1, turn right
- B1: PC13, start / restart

## Controls

- `B1`: start the game from the start screen, or restart after game over
- `SW1`: turn left relative to the current snake direction
- `SW4`: turn right relative to the current snake direction

## Build

Open `NucleoSnake.uvprojx` in Keil uVision and build target `Nucleo_One`.

Command-line build, if `UV4.exe` is available:

```bat
UV4.exe -b "NucleoSnake.uvprojx" -t "Nucleo_One"
```

## Project Structure

- `main.c`: game loop, Snake logic, OLED rendering, timing and clock setup
- `buttons.c` / `buttons.h`: button GPIO setup and button state helpers
- `i2c1.c` / `i2c1.h`: I2C1 communication for the OLED
- `oled.c` / `oled.h`: SSD1306 OLED helper functions
- `font6x8.h`: small bitmap font used for text rendering
- `RTE/`: Keil/CMSIS startup and device support files

## Technical Notes

- The OLED is rendered through a framebuffer: `framebuffer[8][128]`.
- The playfield uses 4x4 pixel cells inside a 1-pixel border.
- The snake body is stored in `snakeX[]` and `snakeY[]`; index `0` is the head.
- Food is placed with a small 16-bit LFSR pseudo-random generator.
- `SysTick_Handler()` increments `ticks` every 1 ms.
- `GAME_STEP_MS` controls snake speed.
- `TURN_DEBOUNCE_MS` avoids duplicate turns from button bounce.
- `turnQueued` prevents multiple turns before the snake has moved one step.

## Methods Used

- Direct register access is used instead of HAL to keep the firmware small and close to the IoT1 course style.
- A framebuffer is used so the game can draw a complete frame in RAM before sending it to the OLED.
- Polling is used for button input because the game loop is simple and does not need interrupt-driven controls.
- SysTick timing is used for movement, delays and animation because it provides a simple 1 ms time base.
- Relative turning is used so only two buttons are needed: left and right from the snake's current direction.
- Debounce and a turn queue are used to prevent one physical button press from causing multiple turns.
- A small LFSR is used for food placement because it is lightweight and does not require extra library support.

## Runtime Behavior

On startup, the OLED shows the Snake start screen. Press `B1` to begin. The snake starts near the middle and moves to the right. Eating food increases the score and length. Hitting the border or the snake body shows `GAME OVER` with the final score.

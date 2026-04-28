# NucleoSnake

Snake game for the STM32F091RC Nucleo board with an SSD1306 OLED display.

This is a clean, modular implementation of the classic Snake game, designed as a side project for the IoT1 course at VIVES Kortrijk.

## Features

- Classic Snake gameplay on a 128x64 OLED display.
- Animated start screen with a small snake tongue flick.
- Relative left/right controls using shield buttons.
- **Refined Game Over screen** with high-contrast inverted elements.
- Clean separation between game logic and hardware drivers.

## Hardware

- **Board:** STM32F091RC Nucleo
- **Display:** SSD1306 OLED (128x64 pixels)
- **OLED SCL:** D15 / PB8
- **OLED SDA:** D14 / PB9
- **SW1 (Left):** PA1
- **SW4 (Right):** PC1
- **B1 (Start/Restart):** PC13 (Blue User Button)

## Build

Open `NucleoSnake.uvprojx` in Keil uVision and build target `Nucleo_One`.

Command-line build:
```bat
UV4.exe -b "NucleoSnake.uvprojx" -t "Nucleo_One"
```

## Project Structure

- `main.c`: Coordinates game flow, user interface, and high-level rendering.
- `snake_engine.c / .h`: Pure game logic (snake movement, collisions, scoring).
- `system_utils.c / .h`: Hardware initialization, clock setup (48MHz), and timing utilities.
- `buttons.c / .h`: Clean abstraction for GPIO button polling.
- `i2c1.c / .h`: Register-level I2C1 driver.
- `oled.c / .h`: SSD1306 display driver with support for text, framebuffers, and inversion.
- `font6x8.h`: 5x7 bitmap font table.

## Technical Notes

- **Decoupled Engine:** The game logic is entirely separate from the hardware, using a `GameState` structure for easy state management.
- **Framebuffer Rendering:** Uses a 1KB local framebuffer (`8 pages x 128 columns`) flushed to the OLED.
- **Pseudo-Randomness:** Food placement is driven by a 16-bit LFSR (Linear Feedback Shift Register).
- **English Standardized:** Code, comments, and architecture have been refactored to follow professional English conventions.
- **Direct Register Access:** Peripherals are controlled via direct CMSIS register manipulation (no HAL) to keep the binary lightweight.

## Controls

- `B1`: Start game / Restart after Game Over.
- `SW1`: Turn left relative to current direction.
- `SW4`: Turn right relative to current direction.

## Methods Used

- **Modularity:** Separation of concerns ensures that hardware drivers can be updated without affecting game logic.
- **Encapsulation:** Global variables are minimized; the game state is passed as a pointer to the engine.
- **Timing:** `SysTick` provides a 1ms timebase for consistent movement speed and debounce handling.
- **Input Queuing:** Prevents rapid button presses from causing illegal snake turns (e.g., turning back on itself).

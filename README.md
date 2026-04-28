# NucleoSnake

Snake game for the STM32F091RC Nucleo board with an SSD1306 OLED display.

This is a clean, modular implementation of the classic Snake game, designed as a side project for the IoT1 course at VIVES Kortrijk.

## Features

- Classic Snake gameplay on a 128x64 OLED display.
- Animated start screen with a small snake tongue flick.
- Relative left/right controls using shield buttons.
- **Refined Game Over screen** with high-contrast inverted elements.
- Clean separation between game logic and hardware drivers.

## Hardware Setup & Wiring

This project uses the STM32F091RC Nucleo-64 board combined with a custom IoT Extension Shield.

### Pinout
| Component | Pin / Port | Function |
| :--- | :--- | :--- |
| **SSD1306 OLED** | D15 / PB8 | I2C1_SCL |
| **SSD1306 OLED** | D14 / PB9 | I2C1_SDA |
| **SW1 (Left)** | PA1 | GPIO Input (Pull-up) |
| **SW4 (Right)** | PC1 | GPIO Input (Pull-up) |
| **B1 (Action)** | PC13 | Blue User Button (Start/Restart) |
| **Power** | 3V3 & GND | 3.3V logic level |

### Board Configuration
- **Power:** The Nucleo is powered via the ST-LINK USB connection.
- **Jumpers:**
  - Ensure `CN2` jumpers are fitted for onboard ST-LINK programming.
  - Ensure `JP6` (IDD) is fitted, otherwise the target MCU will not receive power.

## Reproduction Checklist

1. **Open the project:** Open `NucleoSnake.uvprojx` in Keil uVision.
2. **Build:** Select target `Nucleo_One` and compile the firmware.
3. **Flash:** Download the compiled binary to the Nucleo board via ST-LINK.
4. **Play:** 
   - The OLED should display the animated NucleoSnake start screen.
   - Press the Blue User Button (`B1`) to start.
   - Use `SW1` and `SW4` to steer the snake.

Command-line build alternative:
```bat
UV4.exe -b "NucleoSnake.uvprojx" -t "Nucleo_One"
```
*(Build outputs and local Keil state are ignored by `.gitignore`.)*

## Project Structure

- `main.c`: Coordinates game flow, user interface, and high-level rendering.
- `snake_engine.c / .h`: Pure game logic (snake movement, collisions, scoring).
- `system_utils.c / .h`: Hardware initialization, clock setup (48MHz), and timing utilities.
- `buttons.c / .h`: Clean abstraction for GPIO button polling.
- `i2c1.c / .h`: Register-level I2C1 driver.
- `oled.c / .h`: SSD1306 display driver with support for text, framebuffers, and inversion.
- `font6x8.h`: 5x7 bitmap font table.

## Technical Notes

- **Direct Register Access:** Peripherals (I2C, GPIO, SysTick) are controlled via direct CMSIS register manipulation (no HAL) in accordance with the IoT1 course conventions.
- **I2C Routing:** PB8 and PB9 are used because they are mapped to the D15/D14 Arduino headers where the OLED display is connected.
- **Decoupled Engine:** The game logic is entirely separate from the hardware, using a `GameState` structure for easy state management.
- **Framebuffer Rendering:** Uses a 1KB local framebuffer (`8 pages x 128 columns`) flushed to the OLED.
- **Pseudo-Randomness:** Food placement is driven by a 16-bit LFSR (Linear Feedback Shift Register).
- **Timing & Input Queuing:** `SysTick` provides a 1ms timebase for consistent movement speed and debounce handling. Input queues prevent rapid button presses from causing illegal snake turns (e.g., turning back on itself).
- **English Standardized:** Code, comments, and architecture have been refactored to follow professional English conventions.

## Reference Documents

- [STM32 Nucleo-64 Boards User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [STM32F091xC Datasheet](https://www.st.com/resource/en/datasheet/stm32f091rc.pdf)
- [STM32F0x1 Reference Manual (RM0091)](https://www.st.com/resource/en/reference_manual/rm0091-stm32f0x1stm32f0x2stm32f0x8-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [SSD1306 OLED Controller Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

## Disclaimer

This project was built with assistance from GPT-5.5 in OpenCode.

## License

This project is released under the MIT License. See `LICENSE` for details.

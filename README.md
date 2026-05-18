# Smart-Safe-PIC18F4455

# Smart Safe / Secure Vault System (PIC18F4455)

A bare-metal embedded system implementing a secure vault locking mechanism. This physical project was developed using a **PIC18F4455** microcontroller (programmed via MPLAB) and simulated in PICSimLab. The system requires a specific 4-digit PIN to actuate a physical locking mechanism, providing visual and auditory feedback.

https://github.com/user-attachments/assets/a340ef40-0053-4221-bd33-50364ea263a8


## Hardware Components
* **Microcontroller:** Microchip PIC18F4455 (Running at 4MHz)
* **Display:** 0.96" OLED SSD1306 (I2C Communication)
* **Input:** 4x4 Matrix Membrane Keypad
* **Actuator:** Micro-Servomotor (Simulating the vault's lock mechanism)
* **Feedback:** Piezo Buzzer (Audio alerts for key presses, success, and error states)

## Software Architecture & Bare-Metal Features
The project is written entirely in **C (Bare-Metal)**, directly manipulating Special Function Registers (SFRs) without external HAL libraries.

* **Hardware Interrupts (Timer0):** Precise PWM signal generation for the Servomotor is handled via Timer0 interrupts, ensuring background execution without blocking the main loop.
* **Custom I2C Implementation:** The I2C protocol for the OLED display is implemented via the MSSP module (`SSPCON2bits`, `SSPBUF`).
* **Custom OLED Rendering:** Instead of using heavy graphic libraries, a custom memory-optimized font array was written to manually render characters ('O', 'P', 'E', 'N', 'C', 'L', 'S', 'D') pixel by pixel to the SSD1306.
* **Matrix Keypad Scanning:** The keypad is polled continuously with debouncing delays, scanning rows (`PORTE`, `PORTA`) and reading columns to map specific characters.
* **Watchdog Timer Management:** Strategic `_asm clrwdt _endasm` instructions are placed inside delay loops and drawing functions to prevent system resets during long hardware operations.

## Pin Mapping / Wiring
| Component | PIC18F4455 Pin | Function |
| :--- | :--- | :--- |
| **OLED SSD1306** | SDA/SCL (Hardware I2C) | Display interface |
| **Servo Motor** | RC0 | PWM Control Signal (Timer0) |
| **Buzzer** | RC1 | Digital Output (Audio) |
| **Keypad Rows** | RE2, RE1, RE0, RA5 | Digital Output (Scanning) |
| **Keypad Cols** | RA4, RA3, RA2, RA1 | Digital Input (Reading) |

## How it Works
1. **Idle State:** The OLED displays `CLOSED` and the servo is positioned at 0 degrees (1.0ms PWM pulse).
2. **Authentication:** The user inputs the 4-digit PIN (default: `4206`). Each keypress generates a short acoustic beep.
3. **Validation:**
   * **Success:** A long beep is triggered, the OLED updates to `OPEN`, and the servo rotates 90 degrees (1.5ms PWM pulse).
   * **Error:** A double-beep error sound is triggered, and the PIN buffer is cleared.
4. **Locking:** Pressing the `#` key immediately resets the system to the `CLOSED` state.

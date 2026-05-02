Embedded Two-Player Mastermind (PSoC 6, FreeRTOS)

This project implements a two-player Mastermind game on the PSoC 6 microcontroller platform, where two physical boards communicate over UART to exchange guesses and feedback in real time.
Each device runs independently while maintaining synchronized game state, demonstrating embedded systems design, real-time task management, and hardware integration.

MCU: Infineon PSoC 6 (ARM Cortex-M4 + M0+)
Development Environment: ModusToolbox
RTOS: FreeRTOS
Peripherals Used
UART – board-to-board communication (game synchronization)
TFT LCD Display – user interface and game feedback
Capacitive Touch Sensor – user input for selecting code/guesses
Ambient Light Sensor – dynamic UI theme switching (light/dark mode)
EEPROM – persistent storage for high scores
Push Buttons (SW1, SW3) – input control and system reset

System Architecture
Designed using FreeRTOS with task-based concurrency
All peripherals accessed through gatekeeper tasks
UART protocol implemented for reliable two-device synchronization
Event-driven system with clear separation between:
UI handling
Communication

Game Flow: 
- Two-Device Synchronization with UART
- Turn-based gameplay with consistent shared state
- Dynamic UI based on ambient lighting (light mode and dark mode)
- Persistent high score tracking using EEPROM
- Touch-based input system for selecting game values
- Robust reset and re-sync mechanism

NOTES:
This project was completed as part of a university embedded systems course.
Starter code was provided; all application logic, communication protocol, and system design were implemented independently.
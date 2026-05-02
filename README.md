Embedded Two-Player Mastermind (PSoC 6, FreeRTOS) 

This project implements a two-player Mastermind game on the PSoC 6 microcontroller platform, where two physical boards communicate over UART to exchange guesses and feedback in real time. 
Each device runs independently while maintaining synchronized game state, demonstrating embedded systems design, real-time task management, and hardware integration. 

<img width="1698" height="1358" alt="image" src="https://github.com/user-attachments/assets/3a799935-871b-41d0-aacd-f899781fec47" />

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
- Devices send discovery messages to each other, when discovery happens, one is selected to go first 
- Before game starts, player can press SW3 to reset high score in EEPROM
- Both players select a cypher using capacitive touch on LCD
- When both players are ready, active player guesses other's cypher
- When guess is submitted, inactive player sends feedback with number of exact matches and digit matches (like wordle)
- Inactive player becomes active player
- If active player wins, both boards transition to game over state
- If active player score < that board's high score, hi-score is updated in EEPROM
- Either player can press button to start game again
- Dynamic UI based on ambient lighting (light mode and dark mode) updates any time
- If one board is reset, other board restarts game
- Robust reset and re-sync mechanism

NOTES:
This project was completed as part of a university-embedded systems course.
Starter code was provided; no given files included
All application logic, communication protocol, and system design were implemented independently.

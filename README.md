# ESP32 FreeRTOS Playground

Learning ESP-IDF and FreeRTOS using ESP32 DevKit V1. Following the Digikey FreeRTOS tutorials.

## Current Progress

### Module 01 – Basic LED Blink
- Setup and configure ESP-IDF environment.
- Created a simple FreeRTOS task to blink the onboard LED.
- Learned:
  - FreeRTOS task creation (`xTaskCreate`)
  - GPIO configuration with ESP32
  - Basic understanding of the scheduler

### Module 02 – Dual Task LED Blink
- Created two separate FreeRTOS tasks to blink the same LED at different rates concurrently.
- Learned:
  - Running multiple tasks in parallel
  - Understanding task priorities
  - Scheduler behavior when tasks compete for CPU
  - Modularizing code for tasks (separate `.c` files for each module)
  
## Board
- ESP32 DevKit V1

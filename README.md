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

### Module 03 – UART-Controlled LED
- Two concurrent FreeRTOS tasks:
  1. LED blink task (rate controlled by global variable)
  2. UART read task (reads integer from Serial Monitor to update LED rate)
- Learned:
  - Inter-task concurrency
  - Shared-variable communication between tasks
  - Implemented using the ESP-IDF UART driver
  - Hardware-to-software data flow in UART:  
  `UART RX → Hardware FIFO → ISR → Driver ring buffer → Application`
  - Blocking reads with timeout in FreeRTOS
  - How ESP-IDF driver APIs differ from high-level console functions (`getchar`, `scanf`)

### Module 04 – FreeRTOS Heap/Memory Management
- Two concurrent FreeRTOS tasks:
  1. UART read task – waits for input, allocates heap memory using pvPortMalloc()
  2. Print task – waits for notification from the UART task, prints the message, frees the heap memory with vPortFree()
- Learned:
  - Dynamic heap allocation and deallocation in FreeRTOS
  - Thread safe communication between tasks
  - configASSERT() ensures allocation succeeded. Halts processor if out of memory.
  - Avoid unsafe direct access to heap pointer before the message is fully copied

### Module 05 – Queues (CLI ↔ LED Control)
- Two concurrent tasks that communicate using two queues:
  **1. Task A**
    - Reads user input through UART
    - Parses the delay and sends integer delay value to Queue 1
    - Checks Queue 2 and prints incoming messages from Task B
  **2. Task B**
    - Blinks the LED
    - Polls Queue 1 for new delay values and updates blink rate
    - Sends a status message every 100 blinks to Queue 2 so that Task A can print it
- Learned:
  - Using queues for thread safe task to task communication instead of shared global values
  - Keep UART/printing mostly in one place- Task A, while Task B focuses on control logic
  - Using non blocking queue receives to keep tasks responsive

## Board
- ESP32 DevKit V1

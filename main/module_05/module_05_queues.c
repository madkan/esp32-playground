#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "common/led_driver.h"

#define UART_BUF_SIZE     1024
#define CLI_BUF_LEN       255
#define COMMAND_PREFIX    "delay "
#define DELAY_QUEUE_LEN   5
#define MSG_QUEUE_LEN     5
#define BLINK_MAX         100

//Message Struct for Queue 2
typedef struct {
    char body[20];
    int  count;
} module05_msg_t;

// Queues
static QueueHandle_t delay_queue = NULL; // Queue 1 - int
static QueueHandle_t msg_queue   = NULL; // Queue 2 =module05_msg_t

// Helpers - UART write
static void uart_write_str(const char *s)
{
    if(!s){
        return;
    }
    uart_write_bytes(UART_NUM_0, s, (int)strlen(s));
}

static void uart_write_int_line(const char *prefix, int value)
{
    char line[64];
    int n = snprintf(line, sizeof(line), "%s%d\r\n", prefix, value);
    if(n > 0){
        uart_write_bytes(UART_NUM_0, line, n);
    } 
}

// UART init
static void uart_init_driver(void)
{
    uart_config_t cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM_0, UART_BUF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &cfg);
}

//Task A - CLI/UART & prints messages from Queue 2
static void task_cli(void *pvParameters)
{
    (void)pvParameters;
    uint8_t rx;
    char buf[CLI_BUF_LEN];
    uint16_t idx = 0;
    const size_t cmd_len = strlen(COMMAND_PREFIX);

    memset(buf, 0, sizeof(buf));
    uart_write_str("\r\n--- Module 05: Queues ---\r\n");
    uart_write_str("Type: delay <ms>  (example: delay 250)\r\n");

    while (1) {
        // 1. Print any new messages from Queue 2 no blocking
        module05_msg_t incoming;
        while (xQueueReceive(msg_queue, &incoming, 0) == pdTRUE) {
            uart_write_str(incoming.body);                           // Print message+count
            uart_write_int_line("", incoming.count);
        }
        // 2. Read 1 byte from UART with timeout
        int len = uart_read_bytes(UART_NUM_0, &rx, 1, pdMS_TO_TICKS(50));
        if (len > 0) {
            char c = (char)rx;
            uart_write_bytes(UART_NUM_0, &c, 1);            // Echo back typed characters
            if (idx < (CLI_BUF_LEN - 1)) {                  // Store into line buffer
                buf[idx++] = c;
            }
            if (c == '\n' || c == '\r') {              // On newline- process the line
                uart_write_str("\r\n");
                // Ensure null terminated string and strip trailing newline
                while (idx > 0 && (buf[idx - 1] == '\n' || buf[idx - 1] == '\r')) {
                    buf[idx - 1] = '\0';
                    idx--;
                }
                if (idx > 0) {
                    // Check for "delay "
                    if (strncmp(buf, COMMAND_PREFIX, cmd_len) == 0) {
                        const char *tail = buf + cmd_len;
                        int new_delay = atoi(tail);
                        if (new_delay < 0) new_delay = -new_delay;
                        if (new_delay == 0) {
                            uart_write_str("Invalid delay. Try: delay 250\r\n");
                        } else {
                            if (xQueueSend(delay_queue, &new_delay, pdMS_TO_TICKS(10)) != pdTRUE) {
                                uart_write_str("ERROR: Could not send to delay queue\r\n");
                            } else {
                                uart_write_int_line("Updated delay to: ", new_delay);
                            }
                        }
                    }
                }
                // Reset input buffer
                memset(buf, 0, sizeof(buf));
                idx = 0;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}


// Task B -  Blink LED & reacts to Queue 1 & sends status to Queue 2
static void task_blink(void *pvParameters)
{
    (void)pvParameters;
    int led_delay_ms = 500;
    uint32_t counter = 0;

    while (1) {
        // Check for a new delay message
        int new_delay;
        if (xQueueReceive(delay_queue, &new_delay, 0) == pdTRUE) {
            led_delay_ms = new_delay;
            module05_msg_t msg = {0};           // Notify Task A via Queue 2
            strncpy(msg.body, "Delay set: ", sizeof(msg.body) - 1);
            msg.count = led_delay_ms;
            xQueueSend(msg_queue, &msg, pdMS_TO_TICKS(10));
        }
        led_on();
        vTaskDelay(pdMS_TO_TICKS(led_delay_ms));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(led_delay_ms));
        // Every 100 blinks - send blinked message
        counter++;
        if (counter >= BLINK_MAX) {
            module05_msg_t msg = {0};
            strncpy(msg.body, "Blinked: ", sizeof(msg.body) - 1);
            msg.count = (int)counter;
            xQueueSend(msg_queue, &msg, pdMS_TO_TICKS(10));
            counter = 0;
        }
    }
}

void module_05_start(void)
{
    uart_init_driver();

    delay_queue = xQueueCreate(DELAY_QUEUE_LEN, sizeof(int));
    msg_queue   = xQueueCreate(MSG_QUEUE_LEN, sizeof(module05_msg_t));

    if (delay_queue == NULL || msg_queue == NULL) {
        uart_write_str("ERROR: Queue create failed\r\n");
        return;
    }
    xTaskCreate(task_cli,   "CLI Task",   4096, NULL, 5, NULL);
    xTaskCreate(task_blink, "Blink Task", 4096, NULL, 5, NULL);
}

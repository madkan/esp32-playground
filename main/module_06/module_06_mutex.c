#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "common/led_driver.h"

#define INPUT_BUF_LEN 64

static SemaphoreHandle_t mutex = NULL;

static void uart_init_driver(void)
{
    uart_config_t cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &cfg);
}

static void uart_write_str(const char *s)
{
    if(!s){
        return;
    }
    uart_write_bytes(UART_NUM_0, s, (int)strlen(s));
}

static int uart_read_line(char *out, int out_len)
{
    if (!out || out_len <= 1) return 0;
    int idx = 0;
    memset(out, 0, out_len);
    while (1) {
        uint8_t c = 0;
        int len = uart_read_bytes(UART_NUM_0, &c, 1, pdMS_TO_TICKS(100));
        if (len <= 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        uart_write_bytes(UART_NUM_0, (const char *)&c, 1);
        if (c == '\n' || c == '\r') {
            uart_write_str("\r\n");
            out[idx] = '\0';
            return idx;
        }
        if (idx < out_len - 1) {
            out[idx++] = (char)c;
        }
    }
}

static void blink_task(void *parameters)
{
    // Copy the parameter into a local variable
    int num = *(int*)parameters;
    // Release mutex
    xSemaphoreGive(mutex);

    char msg[64];
    snprintf(msg, sizeof(msg), "Received: %d ms\r\n", num);
    uart_write_str(msg);
    if (num <= 0) num = 500;
    while (1) {
        led_on();
        vTaskDelay(pdMS_TO_TICKS(num));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(num));
    }
}

void module_06_start(void)
{
    uart_init_driver();
    uart_write_str("\r\n--- Module 06: FreeRTOS Mutex ---\r\n");
    uart_write_str("Enter delay in ms (example: 250) and press Enter:\r\n");

    // local variable as argument
    int delay_arg = 0;

    char line[INPUT_BUF_LEN];
    uart_read_line(line, sizeof(line));
    delay_arg = atoi(line);
    char msg[64];
    snprintf(msg, sizeof(msg), "Sending: %d ms\r\n", delay_arg);
    uart_write_str(msg);

    // Create mutex before starting task
    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL) {
        uart_write_str("ERROR: Failed to create mutex\r\n");
        return;
    }
    // start task- pass pointer to stack variable
    xTaskCreate(blink_task, "Blink Task", 4096, (void*)&delay_arg, 5, NULL);
    xSemaphoreTake(mutex, portMAX_DELAY);
    uart_write_str("Done!\r\n");
}

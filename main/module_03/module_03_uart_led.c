#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "common/led_driver.h"

#define UART_BUF_SIZE 256
#define LED_INPUT_BUF 20

static int led_delay = 500;

static void led_blink(void *pvParameters)
{
    while (1) {
        led_on();
        vTaskDelay(pdMS_TO_TICKS(led_delay));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(led_delay));
    }
}

static void uart_read(void *pvParameters)
{
    uint8_t buf[LED_INPUT_BUF];
    uint8_t idx = 0;
    char c;
    memset(buf, 0, LED_INPUT_BUF);
    while (1) {
        int len = uart_read_bytes(UART_NUM_0, (uint8_t*)&c, 1, pdMS_TO_TICKS(100));

        if (len > 0) {
            if (c == '\n' || c == '\r') {
                if (idx > 0) {
                    buf[idx] = '\0';
                    int val = atoi((char*)buf);
                    if (val > 0 && val < 10000) {
                        led_delay = val;
                        printf("Updated LED delay to: %d ms\n", led_delay);
                    } else {
                        printf("Invalid value: %d\n", val);
                    }

                    idx = 0;
                    memset(buf, 0, LED_INPUT_BUF);
                }
            } else if (idx < LED_INPUT_BUF - 1) {
                buf[idx++] = c;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void module_03_start(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, UART_BUF_SIZE, 0, 0, NULL, 0);

    xTaskCreate(led_blink, "LED Blink", 2048, NULL, 5, NULL);
    xTaskCreate(uart_read, "UART Read", 2048, NULL, 5, NULL);
}

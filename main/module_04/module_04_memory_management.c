#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

#define INPUT_BUF_SIZE  1024
#define INPUT_BUF_LEN   255

static TaskHandle_t g_print_task = NULL;
static char *g_msg_ptr = NULL;          // heap pointer
static size_t g_msg_len = 0;

static void uart_write_str(const char *s)
{
    if(!s){
        return;
    }
    uart_write_bytes(UART_NUM_0, s, (int)strlen(s));
}


// Task 1 - UART read task
// Builds a line in a local buffer
// On newline: allocates heap exactly sized, copies string
// Notifies print task when the heap buffer is ready
static void uart_read_task(void *pvParameters)
{
    (void)pvParameters;
    char line_buf[INPUT_BUF_LEN];
    size_t idx = 0;
    uint8_t rx = 0;
    memset(line_buf, 0, sizeof(line_buf));
    uart_write_str("\r\n--- Module 04: FreeRTOS Heap Demo ---\r\n");
    uart_write_str("Type a line and press Enter.\r\n");
    while (1) {
        int len = uart_read_bytes(UART_NUM_0, &rx, 1, pdMS_TO_TICKS(50));
        if(len > 0){
            char c = (char)rx;
            uart_write_bytes(UART_NUM_0, &c, 1);
            if(c == '\r' || c == '\n'){
                if(idx > 0){
                    line_buf[idx] = '\0';
                    if (g_msg_ptr == NULL) {
                        size_t alloc_len = idx + 1;
                        char *p = (char *)pvPortMalloc(alloc_len);
                        configASSERT(p);
                        memcpy(p, line_buf, alloc_len);
                        g_msg_ptr = p;
                        g_msg_len = alloc_len;
                        xTaskNotifyGive(g_print_task);
                    }
                    else{
                        uart_write_str("\r\n(Dropped: previous message not printed yet)\r\n");
                    }
                }
                memset(line_buf, 0, sizeof(line_buf));
                idx = 0;
                uart_write_str("\r\n");
            }
            else{
                if(idx < (INPUT_BUF_LEN - 1)){
                    line_buf[idx++] = c;
                }
                else{
                    uart_write_str("\r\n(Input too long; buffer reset)\r\n");
                    memset(line_buf, 0, sizeof(line_buf));
                    idx = 0;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Task 2 - Print task
// Waits for notification
// Prints heap message
// Frees heap and clears pointer
static void print_task(void *pvParameters)
{
    (void)pvParameters;
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        char *local = g_msg_ptr;
        size_t local_len = g_msg_len;
        g_msg_ptr = NULL;
        g_msg_len = 0;
        if (local != NULL) {
            uart_write_str("Received message: ");
            uart_write_bytes(UART_NUM_0, local, (int)strnlen(local, local_len));
            uart_write_str("\r\n");
            char tmp[96];
            int n = snprintf(tmp, sizeof(tmp), "Free heap before free: %u bytes\r\n",
                            (unsigned)xPortGetFreeHeapSize());
            if (n > 0) uart_write_bytes(UART_NUM_0, tmp, n);

            vPortFree(local);

            n = snprintf(tmp, sizeof(tmp), "Free heap after free:  %u bytes\r\n",
                        (unsigned)xPortGetFreeHeapSize());
            if (n > 0) uart_write_bytes(UART_NUM_0, tmp, n);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void module_04_start(void)
{
    uart_config_t cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM_0, INPUT_BUF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &cfg);
    
    xTaskCreate(print_task, "Print Task", 4096, NULL, 5, &g_print_task);
    xTaskCreate(uart_read_task, "UART Read Task", 4096, NULL, 5, NULL);
}
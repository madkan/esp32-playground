#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common/led_driver.h"

static void led_blink(void *pvParameters)
{
    while (1) {
        led_on();
        vTaskDelay(pdMS_TO_TICKS(500));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void module_01_start(void)
{
    xTaskCreate(
        led_blink,
        "led_blink",
        2048,
        NULL,
        1,
        NULL
    );
}
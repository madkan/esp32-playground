#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#define LED_GPIO GPIO_NUM_12

void led_blink_task(void *pvParameters);

void app_main(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    xTaskCreate(
        led_blink_task,
        "led_blink",
        2048,
        NULL,
        5,
        NULL
    );
    // printf("Hello world!\n");
}

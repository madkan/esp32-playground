#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common/led_driver.h"

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const int rate_1 = 500;
static const int rate_2 = 323;


static void led_slow(void *pvParameters)
{
    while (1) {
        led_on();
        vTaskDelay(pdMS_TO_TICKS(rate_1));

        led_off();
        vTaskDelay(pdMS_TO_TICKS(rate_1));
    }
}

static void led_fast(void *pvParameters)
{
    while (1) {
        led_on();
        vTaskDelay(pdMS_TO_TICKS(rate_2));

        led_off();
        vTaskDelay(pdMS_TO_TICKS(rate_2));
    }
}

void module_02_start(void)
{
    xTaskCreatePinnedToCore(
        led_fast,
        "led_fast",
        2048,
        NULL,
        1,
        NULL,
        app_cpu
    );

    xTaskCreatePinnedToCore(
        led_slow,
        "led_slow",
        2048,
        NULL,
        1,
        NULL,
        app_cpu
    );
}

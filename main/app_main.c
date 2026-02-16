#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "common/led_driver.h"

#define RUN_MODULE_01  1
#define RUN_MODULE_02  0

#if RUN_MODULE_01
#include "module_01/module_01_led_blink.h"
#endif

#if RUN_MODULE_02
#include "module_02/module_02_dual_led.h"
#endif

void app_main(void)
{
    led_init();
#if RUN_MODULE_01
    module_01_start();
#endif

#if RUN_MODULE_02
    module_02_start();
#endif
}

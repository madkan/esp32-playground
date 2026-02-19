#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "common/led_driver.h"

#define RUN_MODULE_01  0
#define RUN_MODULE_02  0
#define RUN_MODULE_03  0
#define RUN_MODULE_04  1

#if RUN_MODULE_01
#include "module_01/module_01_led_blink.h"
#endif

#if RUN_MODULE_02
#include "module_02/module_02_dual_led.h"
#endif

#if RUN_MODULE_03
#include "module_03/module_03_uart_led.h"
#endif

#if RUN_MODULE_04
#include "module_04/module_04_memory_management.h"
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

#if RUN_MODULE_03
    module_03_start();
#endif

#if RUN_MODULE_04
    module_04_start();
#endif
}

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_event.h"


#include "esp_nimble_hci.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "sdkconfig.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include <stdio.h>
#include <time.h>
#include <ctype.h> 


#include "motor_control.h"
#include "led_control.h"
#include "ble_control.h"
#include "comms.h"

#define INIT_TIME_MS 5000

void ble_task(void *p)
{
    connect_ble();     // läuft nie zurück, blockiert aber nur EINE Task
    vTaskDelete(NULL); // Wird nie erreicht, aber formal OK
}

void app_main()
{

    comms_init(); // Task kommunikation
    motor_control();
    // xTaskCreate(motor_control, "MOTOR_TASK", 2048, NULL, 6, NULL);
    xTaskCreate(leds_control, "LED_TASK", 2048, NULL, 6, NULL);
    vTaskDelay(INIT_TIME_MS);
    xTaskCreate(ble_task, "BLE_TASK", 4096, NULL, 6, NULL);
    
}

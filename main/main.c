#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_event.h"


#include "motor_control.h"
#include "led_control.h"
#include "ble_control.h"
#include "comms.h"

#define INIT_TIME_MS 5000

void ble_task(void *p)
{
    connect_ble();     
    vTaskDelete(NULL); 
}

void app_main()
{

    comms_init(); // Task kommunikation - initialisieren 
    motor_control();
    xTaskCreate(leds_control, "LED_TASK", 2048, NULL, 6, NULL);
    vTaskDelay(INIT_TIME_MS);
    xTaskCreate(ble_task, "BLE_TASK", 4096, NULL, 6, NULL);
    
}

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_event.h"


#include "shared_adc.h"
#include "shared_comms.h"
#include "motor_control.h"
#include "ble_control.h"
#include "sensors_control.h"

#define INIT_TIME_MS 500

void ble_task(void *p)
{
    connect_ble();     
    vTaskDelete(NULL); 
}



void app_main()
{
    adc_shared_init();     
    comms_init(); 
    xTaskCreate(ble_task, "BLE_TASK", 4096, NULL, 6, NULL);
    vTaskDelay(INIT_TIME_MS);
    motor_control();
    sensor_control_init();
    sensor_event_t event;
    int motor_command = 0;

    while (1) {
        motor_command = 0;
        if (xQueueReceive(sensor_queue, &event, portMAX_DELAY)) {
            if (event.sensor == 2){ // Taster Auf Event
                if (event.state == true){ // Taster Auf Event
                    ESP_LOGI("MAIN_C", "Sensor_Taster_Auf betätigt: %d ", event.state);
                    motor_command = 2;
                    xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
                }
            }
            if (event.sensor == 3){ // Taster Ab Event
                if (event.state == true){ // Taster Ab Event
                    ESP_LOGI("MAIN_C", "Sensor_Taster_Ab betätigt: %d ", event.state);
                    motor_command = 3;
                    xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
                }
            }
            if (event.sensor == 4){ // Rollenschalter Oben Event
                if (event.state == true){ // Rollenschalter oben aktiv Event
                    ESP_LOGI("MAIN_C", "Sensor_Rollenschalter oben: %d ", event.state);
                    motor_command = 4;
                    xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
                } else {
                    ESP_LOGI("MAIN_C", "Sensor_Rollenschalter oben: %d ", event.state);
                    motor_command = 41;
                    xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
                }
            }
            
            if (event.sensor == 5){ // Rollenschalter Unten Event
                if (event.state == true){ // Rollenschalter Unten Event
                    ESP_LOGI("MAIN_C", "Sensor_Rollenschalter unten: %d ", event.state);
                    motor_command = 5;
                    xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
                } else {
                    ESP_LOGI("MAIN_C", "Sensor_Rollenschalter unten: %d ", event.state);
                    motor_command = 51;
                    xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
                }
            }
            if (event.sensor == 0){ // Windevent Event
                if (event.state == true){ // aktive Event
                    ESP_LOGI("MAIN_C", "Sensor_Wind-: speed: %f, avg: %f ", event.wind_speed, event.wind_avg);
                    float wind_avg = event.wind_avg;
                    if (wind_avg > 70){
                        motor_command = 6;
                        xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
                    } else {
                        motor_command = 61;
                        xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
                    }
               }
            }
            // if (event.sensor == 1){ // Regen Event
            //     if (event.state == true){ // Regen aktiv Event
            //         ESP_LOGI("MAIN_C", "Sensor_Regen aktiv: %d ", event.state);
            //         motor_command = 7;
            //         xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
            //     } else {
            //         ESP_LOGI("MAIN_C", "Sensor_Regen aktiv: %d ", event.state);
            //         motor_command = 71;
            //         xQueueSend(speed_queue, &motor_command, portMAX_DELAY);
            //     }
            // }
        }
        
        // xTaskCreate(leds_control, "LED_TASK", 2048, NULL, 6, NULL);
    }
}





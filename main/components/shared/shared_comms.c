/*
* Queues öffentlich machen, alles Programm greifen auf dieses Queues zu.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "shared_comms.h"
QueueHandle_t led_queue;
QueueHandle_t steering_queue;
QueueHandle_t speed_queue;
QueueHandle_t sensor_queue;

message_speed_t message_speed;

void comms_init(void){
    led_queue = xQueueCreate(10, sizeof(int));
    steering_queue = xQueueCreate(10, sizeof(int));
    speed_queue = xQueueCreate(10, sizeof(message_speed));
    sensor_queue = xQueueCreate(20, sizeof(int));
}

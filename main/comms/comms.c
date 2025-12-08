/*
* Queues öffentlich machen, alles Programm greifen auf dieses Queues zu.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "comms.h"
QueueHandle_t led_queue;
QueueHandle_t steering_queue;
QueueHandle_t acc_queue;

message_acc_type message_acc;

void comms_init(void){
    led_queue = xQueueCreate(10, sizeof(int));
    steering_queue = xQueueCreate(10, sizeof(int));
    acc_queue = xQueueCreate(10, sizeof(message_acc));
}

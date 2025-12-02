#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "comms.h"
QueueHandle_t my_queue;
QueueHandle_t steering_queue;
QueueHandle_t acc_queue;
QueueHandle_t speed_queue;


void comms_init(void){
    my_queue = xQueueCreate(10, sizeof(int));
    speed_queue = xQueueCreate(100, sizeof(int));
    steering_queue = xQueueCreate(10, sizeof(int));
    acc_queue = xQueueCreate(10, sizeof(int));
}

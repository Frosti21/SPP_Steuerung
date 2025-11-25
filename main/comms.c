#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "comms.h"
QueueHandle_t my_queue;

void comms_init(void){
    my_queue = xQueueCreate(10, sizeof(int));
}

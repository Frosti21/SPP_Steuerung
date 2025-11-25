#pragma once
#include "freertos/queue.h"

extern QueueHandle_t my_queue;

void comms_init(void);

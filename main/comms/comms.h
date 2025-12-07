#ifndef COMMS_H
#define COMMS_H

typedef struct {
    uint8_t type;
    uint16_t value;
} message_acc_type;

extern message_acc_type message_acc; 
extern QueueHandle_t my_queue;
extern QueueHandle_t steering_queue;
extern QueueHandle_t acc_queue;
extern QueueHandle_t speed_queue;


void comms_init(void);
#endif // COMMS


#ifndef COMMS
#define COMMS


extern QueueHandle_t my_queue;
extern QueueHandle_t steering_queue;
extern QueueHandle_t acc_queue;

void comms_init(void);
#endif // COMMS


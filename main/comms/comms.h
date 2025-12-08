#ifndef COMMS_H
#define COMMS_H

// message_acc_type -> für die Geschwindigkeits kommunikation, zwei Funktionen über die selbe Queue
// Type -> Geschwindigkeitswerte oder Richtung (Vor-Zurück)
// Value -> Geschwindigkeit (von Poti) oder Vor/Zurück

typedef struct {
    uint8_t type;
    uint16_t value;
} message_acc_type;

extern message_acc_type message_acc; 

extern QueueHandle_t led_queue;
extern QueueHandle_t steering_queue;
extern QueueHandle_t acc_queue;

void comms_init(void);
#endif // COMMS


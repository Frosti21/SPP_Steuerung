#ifndef BLE_CONTROL
#define BLE_CONTROL

extern QueueHandle_t my_queue;

void ble_app_advertise(void);
void ble_app_on_sync(void);
void host_task(void*);
void connect_ble(void);
void comms_init(void);


#endif // BLE_CONTROL


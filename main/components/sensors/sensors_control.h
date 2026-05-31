#ifndef SENSORS_CONTROL_H
#define SENSORS_CONTROL_H



#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdbool.h>
#include <stdint.h>

/* ============================================================
 * SENSOR_CONTROL.H
 * ============================================================ */

/* Sensor IDs */
typedef enum {
    SENSOR_WIND         = 0,
    SENSOR_REGEN        = 1,
    SENSOR_TASTER_AUF   = 2,
    SENSOR_TASTER_AB    = 3,
    SENSOR_END_OBEN     = 4,
    SENSOR_END_UNTEN    = 5,
} sensor_id_t;

/* Event das in die Queue kommt */
typedef struct {
    sensor_id_t sensor;     // welcher Sensor
    bool        state;      // neuer Zustand (true = aktiv/ausgelöst)
    float       wind_speed; // nur relevant wenn sensor == SENSOR_WIND
    float       wind_avg;   // nur relevant wenn sensor == SENSOR_WIND
} sensor_event_t;

/* Öffentliche Queue – von aussen lesbar */
extern QueueHandle_t sensor_queue;

/* ============================================================
 * Initialisierung
 * Einmalig in app_main() aufrufen
 * ============================================================ */
void sensor_control_init(void);

#endif // SENSORS_CONTROL_H

#ifndef SENSORS_H
#define SENSORS_H

#include <stdbool.h>
#include <stdint.h>

void  sensors_init(void);
void  wind_sensor_update(void);

float wind_get_speed(void);
float wind_get_average(void);

bool  rain_is_active(void);
bool  button_up_is_pressed(void);
bool  button_down_is_pressed(void);
bool  end_switch_top_triggered(void);
bool  end_switch_bottom_triggered(void);
#endif // SENSORS_H

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "esp_adc/adc_oneshot.h"

// Shared ADC Handle – nutzbar von anderen Komponenten (z.B. sensors)
extern adc_oneshot_unit_handle_t motor_adc_handle;


void motor_init(void);
void motor_A_coast(void);
void motor_A_stop(void);
void motor_A_set(int speed);

void motor_B_coast(void);
void motor_B_stop(void);
void motor_B_set(int speed);

#endif // MOTOR_H

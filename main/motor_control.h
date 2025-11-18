#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
#include "motor.h"

int control_init(void);

// Motor A
void forward(uint8_t seconds);
void backward(uint8_t seconds);
void stop(void);

// Motor B
void left(uint8_t seconds);
void right(uint8_t seconds);

#endif // MOTOR_CONTROL_H

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);

// Motor A
void motorA_for(uint8_t seconds);
void motorA_back(uint8_t seconds);
void motorA_break(void);

// Motor B
void motorB_for(uint8_t seconds);
void motorB_back(uint8_t seconds);
void motorB_break(void);

#endif // MOTOR_H

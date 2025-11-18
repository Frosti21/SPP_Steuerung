#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);
void motor_coast(int motor);
void motor_stop(int motor);
void motor_set(int motor, int speed);

#endif // MOTOR_H

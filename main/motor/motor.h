#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);
void motor_A_coast(int motor);
void motor_A_stop(int motor);
void motor_A_set(int motor, int speed);

void motor_B_coast(int motor);
void motor_B_stop(int motor);
void motor_B_set(int motor, int speed);

#endif // MOTOR_H

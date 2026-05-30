#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);
void motor_A_coast(void);
void motor_A_stop(void);
void motor_A_set(int speed);

void motor_B_coast(void);
void motor_B_stop(void);
void motor_B_set(int speed);

#endif // MOTOR_H

#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>

void configure_led(void);

void led_right(void);
void led_forward(void);
void led_backward(void);
void led_left(void);
void led_off(void);
void led_break(void);


#endif // LEDS_H

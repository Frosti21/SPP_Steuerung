#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#include "motor.h"
#include <stdio.h>

// Pinbelegung Motoren
#define MOTOR_A_IN1_PIN    0
#define MOTOR_A_IN2_PIN    1
#define MOTOR_A_PWM_PIN    9 // PWM_PIN A1, OI9

#define MOTOR_B_IN1_PIN    5
#define MOTOR_B_IN2_PIN    6
#define MOTOR_B_PWM_PIN    7

// PWM-Konfiguration
#define LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LEDC_TIMER         LEDC_TIMER_0
#define LEDC_FREQ_HZ       20000
#define LEDC_DUTY_RES      LEDC_TIMER_10_BIT
#define LEDC_MAX_DUTY      ((1 << LEDC_DUTY_RES) - 1)

#define LEDC_CHANNEL_A     LEDC_CHANNEL_0
#define LEDC_CHANNEL_B     LEDC_CHANNEL_1


// Initialisierung der PINs PWM usw... 
void motor_init(void)
{
    // Timer konfigurieren
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // PWM-Kanal Motor A
    ledc_channel_config_t ledc_channel_a = {
        .gpio_num       = MOTOR_A_PWM_PIN,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_A,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0,
        .flags.output_invert = 1
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_a));

    // PWM-Kanal Motor B
    ledc_channel_config_t ledc_channel_b = {
        .gpio_num       = MOTOR_B_PWM_PIN,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_B,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0,
        .flags.output_invert = 1
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_b));

    // Richtungs-Pins konfigurieren
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << MOTOR_A_IN1_PIN) |
                        (1ULL << MOTOR_A_IN2_PIN) |
                        (1ULL << MOTOR_B_IN1_PIN) |
                        (1ULL << MOTOR_B_IN2_PIN)
    };
    gpio_config(&io_conf);
}
// Motor Ein - A
void motor_A_set(int motor, int speed)
{
    int dir1, dir2;
    int duty = abs(speed);
    dir1 = MOTOR_A_IN1_PIN;
    dir2 = MOTOR_A_IN2_PIN;
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A);


    if (speed > 0) {
        gpio_set_level(dir1, 1);
        gpio_set_level(dir2, 0);
    } else if (speed < 0) {
        gpio_set_level(dir1, 0);
        gpio_set_level(dir2, 1);
    } else {
        gpio_set_level(dir1, 0);
        gpio_set_level(dir2, 0);
    }
}

void motor_A_stop(int motor)
{
    gpio_set_level(MOTOR_A_IN1_PIN, 1);
    gpio_set_level(MOTOR_A_IN2_PIN, 1);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A );
}

// Motor Ein - B
void motor_B_set(int motor, int speed)
{
    int dir1, dir2;
    int duty = abs(speed);
    dir1 = MOTOR_B_IN1_PIN;
    dir2 = MOTOR_B_IN2_PIN;
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B);

    if (speed > 0) {
        gpio_set_level(dir1, 1);
        gpio_set_level(dir2, 0);
    } else if (speed < 0) {
        gpio_set_level(dir1, 0);
        gpio_set_level(dir2, 1);
    } else {
        gpio_set_level(dir1, 0);
        gpio_set_level(dir2, 0);
    }
}

void motor_B_stop(int motor)
{
    gpio_set_level(MOTOR_B_IN1_PIN, 1);
    gpio_set_level(MOTOR_B_IN2_PIN, 1);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B);
}
// Freilauf A
void motor_A_coast(int motor)
{
    // beide LOW -> Freilauf
    gpio_set_level(MOTOR_A_IN1_PIN, 0);
    gpio_set_level(MOTOR_A_IN2_PIN, 0);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A);
}

// Freilauf B
void motor_B_coast(int motor)
{
    // beide LOW -> Freilauf
    gpio_set_level(MOTOR_B_IN1_PIN, 0);
    gpio_set_level(MOTOR_B_IN2_PIN, 0);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B);
}
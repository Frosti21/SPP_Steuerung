#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#include <stdio.h>
#include "motor_control.h"
#include "comms.h"
#include "motor.h"

#define LEDC_DUTY_RES      LEDC_TIMER_10_BIT
#define LEDC_MAX_DUTY      ((1 << LEDC_DUTY_RES) - 1)



static void forward(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A forward");
    // motor_set(0, -LEDC_MAX_DUTY / 2);
    motor_set(0, -100);
    vTaskDelay(pdMS_TO_TICKS(seconds * 1000));
    motor_stop(0);
    // motor_coast(0);

}

static void backward(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A backward");
    // motor_set(0, (LEDC_MAX_DUTY / 2)); LEDC_MAX_DUTY -> 1024
    motor_set(0, (500));
    vTaskDelay(pdMS_TO_TICKS(seconds * 1000));
    motor_stop(0);
}

static void stop(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A bremse");
    motor_stop(0);
    ESP_LOGI("MOTOR_CONTROL", "Motor B bremse");
    motor_stop(1);
}

static void shutdown(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A freilauf");
    motor_coast(0);
    ESP_LOGI("MOTOR_CONTROL", "Motor B freilauf");
    motor_stop(1);

}

void left(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor B left");
    // motor_set(1, LEDC_MAX_DUTY / 2);
    // motor_set(nr, PWM_lengt_up/down);
    motor_set(1, (200));
    vTaskDelay(pdMS_TO_TICKS(seconds * 2000));
    // motor_stop(1);
    // motor_coast(1);
    motor_stop(1);

}

void right(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor B right");
    // motor_set(1, -(LEDC_MAX_DUTY / 2));
    // motor_set(nr, PWM_lengt_up/down);
    motor_set(1, -(200));
    vTaskDelay(pdMS_TO_TICKS(seconds * 2000));
    // motor_coast(1);
    motor_stop(1);
}


static void motor_control_init(void){
    ESP_LOGI("MOTOR_CONTROL", "Start Init");
    motor_init();
    left(1);
    vTaskDelay(pdMS_TO_TICKS(2 * 1000));
    left(3);
    vTaskDelay(pdMS_TO_TICKS(2 * 1000));
    right(3);
    ESP_LOGI("MOTOR_CONTROL", "Ende Init");
}

void motor_control(void*)
{
    motor_control_init();
    int rx = 0;
    uint8_t value = 1;
    while (1) {
        if (xQueueReceive(my_queue, &rx, portMAX_DELAY)) {
            printf("Empfangen: %d\n", rx);
            switch (rx)
            {
            case 1:
                forward(value);
                break;
            case 2:
                backward(value);
                break;
            case 3:
                stop();
                break;
            case 4:
                left(value);        
                break;
            case 5:
                right(value);
                break;
            case 6:
                shutdown();
                break;
            default:
            shutdown();
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
}


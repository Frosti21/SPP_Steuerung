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
    motor_A_set(0, -100);
    //vTaskDelay(pdMS_TO_TICKS(seconds * 1000));
    // motor_A_coast(0);
    // motor_coast(0);

}

static void backward(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A backward");
    // motor_set(0, (LEDC_MAX_DUTY / 2)); LEDC_MAX_DUTY -> 1024
    motor_A_set(0, (500));
    // vTaskDelay(pdMS_TO_TICKS(seconds * 1000));
    // motor_A_coast(0);
}

static void stop(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A bremse");
    motor_A_stop(0);
    ESP_LOGI("MOTOR_CONTROL", "Motor B bremse");
    motor_A_stop(1);
}

static void shutdown(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A freilauf");
    motor_A_coast(0);
    ESP_LOGI("MOTOR_CONTROL", "Motor B freilauf");
    motor_A_stop(1);

}

void left(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor B left");
    // motor_set(1, LEDC_MAX_DUTY / 2);
    // motor_set(nr, PWM_lengt_up/down);
    motor_B_set(1, (200));
    // vTaskDelay(pdMS_TO_TICKS(seconds * 2000));
    // motor_stop(1);
    // motor_coast(1);
    // motor_B_stop(1);

}

void right(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor B right");
    // motor_set(1, -(LEDC_MAX_DUTY / 2));
    // motor_set(nr, PWM_lengt_up/down);
    motor_B_set(1, -200);
    // vTaskDelay(pdMS_TO_TICKS(seconds * 2000));
    // motor_coast(1);
    // motor_B_stop(1);
}


void steering_control(void*)
{
    // motor_control_init();
    int rx = 0;
    uint8_t value = 1;
    while (1) {
        if (xQueueReceive(steering_queue, &rx, portMAX_DELAY)) {
            printf("Steering - Empfangen: %d\n", rx);
            switch (rx)
            {
            case 3:
                stop();
                break;
            case 4:
                left(value);        
                break;
            case 5:
                right(value);
            default:
            shutdown();
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
}


void acc_control(void*)
{
    // motor_control_init();
    int rx = 0;
    uint8_t value = 1;
    while (1) {
        if (xQueueReceive(acc_queue, &rx, portMAX_DELAY)) {
            printf("Acc - Empfangen: %d\n", rx);
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
            case 6:
                shutdown();
                break;
            default:
            shutdown();
                break;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
}



void motor_control(void){

    ESP_LOGI("MOTOR_CONTROL", "Start Init");
    motor_init();
    xTaskCreate(steering_control, "STEERING_TASK", 2048, NULL, 7, NULL);
    xTaskCreate(acc_control, "ACC_TASK", 2048, NULL, 7, NULL);
    ESP_LOGI("MOTOR_CONTROL", "Ende Init");
}


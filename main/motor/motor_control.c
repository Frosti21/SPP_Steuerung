#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#include <stdio.h>
#include "motor_control.h"
#include "comms.h"

static uint16_t speed = 500;

static void forward(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A forward");
    motor_A_set(0, -speed);
}

static void backward(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A backward");
    motor_A_set(0, speed);
}

static void stop(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A bremse");
    motor_A_stop(1);
}

static void steer_neutral(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor B Neutral");
    motor_B_stop(0);
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
    motor_B_set(1, 500);
}

void right(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor B right");
    motor_B_set(1, -500);
}

// Links/Rechts 
void steering_control(void*)
{
    int rx = 0;
    int8_t value = 0;
    while (1) {
        if (xQueueReceive(steering_queue, &rx, portMAX_DELAY)) {
            printf("Steering - Empfangen: %d\n", rx);
            switch (rx)
            {
            case 3:
                stop();
                break;
            case 4:
            if (value < 0){
                steer_neutral();
                value = 0;
            } else{
                value = 1;
                left(1);        
                }
                break;
            case 5:
            if (value > 0){
                steer_neutral();
                value = 0;
            } else{
                value = -1;
                right(1);
            }
            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// Geschwindigkeits Kontrolle
// Vor/Zurück bzw. Geschwindigkeit
void speed_control(void*)
{
    int8_t value = 0;
    message_speed_t ry;
    while (1) {
        if (xQueueReceive(speed_queue, &ry, portMAX_DELAY)) {
            printf("SPEED_Queue - Empfangen: %d\n", ry.type);
            if (ry.type == 1){
                switch (ry.value)
                {
                case 1:
                if (value < 0){
                    stop();
                    value = 0;
                } else {
                    value = 1;
                    forward(1);
                }
                    break;
                case 2:
                if (value > 0){
                    stop();
                    value = 0;
                } else {
                    value = -1;
                    backward(1);
                }
                    break;
                case 3:
                    stop();
                    break;
                case 6:
                    shutdown();
                    break;
                default:
                    break;
                }
            }else if (ry.type == 2){
                if ((ry.value < 2000)){
                    speed = 400;
                } else if ((ry.value < 2500)){
                    speed = 600;
                } else if ((ry.value < 3000) ){
                    speed = 800;
                } else if ((ry.value < 3500)){
                    speed = 1000;
                } else {
                    speed = 500;
                }
            }
            
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// Initialisierung der Motorfunktionstasks
void motor_control(void){
    ESP_LOGI("MOTOR_CONTROL", "Start Init");
    motor_init();
    xTaskCreate(steering_control, "STEERING_TASK", 2048, NULL, 7, NULL);
    xTaskCreate(speed_control, "SPEED_TASK", 2048, NULL, 7, NULL);
    ESP_LOGI("MOTOR_CONTROL", "Ende Init");
}


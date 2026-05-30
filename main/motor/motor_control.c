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

static uint16_t speed = 1023;

static void forward(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A forward");
    motor_A_set(-speed);
}

static void backward(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A backward");
    motor_A_set(speed);
}

static void stop(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A bremse");
    motor_A_stop();
}

static void shutdown(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A freilauf");
    motor_A_coast();
}

// Geschwindigkeits Kontrolle
void speed_control(void*)
{
    int8_t value = 0;
    message_speed_t ry;
    while (1) {
        if (xQueueReceive(speed_queue, &ry, portMAX_DELAY)) {
            printf("SPEED_Queue - Empfangen: %d\n", ry.type);
            if (ry.type == 1) {
                switch (ry.value)
                {
                case 1:
                    if (value < 0) {
                        stop();
                        value = 0;
                    } else {
                        value = 1;
                        forward(1);
                    }
                    break;
                case 2:
                    if (value > 0) {
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
            } else if (ry.type == 2) {
                if (ry.value < 2000) {
                    speed = 400;
                } else if (ry.value < 2500) {
                    speed = 600;
                } else if (ry.value < 3000) {
                    speed = 800;
                } else if (ry.value < 3500) {
                    speed = 1000;
                } else {
                    speed = 500;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// Initialisierung
void motor_control(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Start Init");
    motor_init();
    xTaskCreate(speed_control, "SPEED_TASK", 2048, NULL, 7, NULL);
    ESP_LOGI("MOTOR_CONTROL", "Ende Init");
}
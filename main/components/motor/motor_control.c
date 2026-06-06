#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#include <stdio.h>
#include "motor_control.h"
#include "shared_comms.h"
#include "motor.h"

static uint16_t speed = 1023;

typedef struct {
    uint8_t enable    : 1;
    uint8_t end_up    : 1;
    uint8_t end_down  : 1;
    uint8_t move_for   : 1;
    uint8_t move_back : 1;
    uint8_t stop      : 1;
    uint8_t wind      : 1;
    uint8_t rain      : 1;
} motor_status_t;

typedef union {
    motor_status_t status;
    uint8_t raw;          
} motor_control_t;

// Verwendung:
motor_control_t motor;


static void forward(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A forward");
    motor_A_set(-speed);
    motor.status.stop      = 0;
    motor.status.move_for  = 1;
    motor.status.move_back = 0;
}

static void backward(uint8_t seconds)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A backward");
    motor_A_set(speed);
    motor.status.stop      = 0;
    motor.status.move_for  = 0;
    motor.status.move_back = 1;

}

static void stop(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A bremse");
    motor_A_stop();
    motor.status.stop      = 1;
    motor.status.move_for  = 0;
    motor.status.move_back = 0;
}

static void shutdown(void)
{
    ESP_LOGI("MOTOR_CONTROL", "Motor A freilauf");
    motor_A_coast();
    motor.status.stop      = 1;
    motor.status.move_for  = 0;
    motor.status.move_back = 0;
}

// Geschwindigkeits Kontrolle
void speed_control(void*)
{
    message_speed_t ry;
    while (1) {
        if (xQueueReceive(speed_queue, &ry, portMAX_DELAY)) {
            ESP_LOGI("MOTOR_CONTROL_C", "Motorbefehl: %d ", ry.type);

            switch (ry.type){
                case 2:
                    if ((motor.status.enable ) && ((motor.status.move_back) || (motor.status.wind))) {
                        ESP_LOGI("MOTOR_CONTROL_C", "Motor - STOP!");
                        stop();
                    } else if ((motor.status.enable ) && (motor.status.end_up  == 0 )) {
                        ESP_LOGI("MOTOR_CONTROL_C", "Motor - Vorwärts!");
                        forward(1);
                    }
                    break;
                case 3:
                    if ((motor.status.enable ) && (motor.status.move_for)){
                        ESP_LOGI("MOTOR_CONTROL_C", "Motor - STOP!");
                        stop();
                    } else if ((motor.status.enable ) && (motor.status.end_down  == 0 )) {
                        ESP_LOGI("MOTOR_CONTROL_C", "Motor - Rückwärts!");
                        backward(1);
                    }
                    break;
                case 4: // Endschalter oben
                    if ((motor.status.enable ) && (motor.status.move_for)){
                        motor.status.end_up = 1;
                        stop();
                    }
                    break;
                
                case 5: // Endschalter unten
                    if ((motor.status.enable ) && (motor.status.move_back)){
                        motor.status.end_down = 1;
                        stop();
                    }
                    break;
                case 6: // Schließen, Aufgrund von Wind
                    if ((motor.status.enable ) && (motor.status.end_down  == 0 )) {
                        ESP_LOGI("MOTOR_CONTROL_C", "Schrim zu Aufgrund von Wind!");
                        motor.status.wind = 1;
                        backward(1);
                    }
                    break;
                case 7: // Schließen, Aufgrund von Regen
                    if ((motor.status.enable ) && (motor.status.end_down  == 0 )) {
                            ESP_LOGI("MOTOR_CONTROL_C", "Schrim zu Aufgrund von Regen!");
                            motor.status.rain = 1;
                            backward(1);
                        }
                    break;
                case 41: // Endschalter oben
                    motor.status.end_up = 0;
                    break;
                case 51: // Endschalter unten
                    motor.status.end_down = 0;
                    break;
                case 61: // Schließen, Aufgrund von Wind
                    motor.status.wind = 0;
                    break;
                case 71: // Schließen, Aufgrund von Regen
                    motor.status.rain = 0;
                    break;
            }
            
        } 
    }
    vTaskDelay(pdMS_TO_TICKS(20));
}


// Initialisierung
void motor_control(void)
{
    motor.raw = 0x00;           // alles auf 0 setzen
    ESP_LOGI("MOTOR_CONTROL", "Start Init");
    motor_init();
    motor.status.enable = 1;  
    xTaskCreate(speed_control, "SPEED_TASK", 2048, NULL, 5, NULL);
    ESP_LOGI("MOTOR_CONTROL", "Ende Init");
}
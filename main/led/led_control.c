

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "comms.h"
#include "leds.h"

void leds_control(void*)
{
    configure_led();
    int rx = 0;
    while (1) {
        if (xQueueReceive(led_queue, &rx, portMAX_DELAY)) {
            printf("Empfangen: %d\n", rx);
            switch (rx)
            {
            case 1:
                led_forward();
                break;
            case 2:
                led_backward();
                break;
            case 3:
                led_break();
                break;
            case 4:
                led_left();        
                break;
            case 5:
                led_right();
                break;
            case 6:
                led_off();
                break;
            default:
                led_off();
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
}
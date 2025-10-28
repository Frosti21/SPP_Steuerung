#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "LEDS";

#define BLINK_GPIO 8

static led_strip_handle_t led_strip;

void led_left(void)
{
    // int blue = 16;
    // int anzahl =  sizeof(lights)/sizeof(lights[0]);  //25; //sizeof(lights)/sizeof(lights[0]);
    for (int i = 0; i < 25; i++){
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    led_strip_set_pixel(led_strip, 0, 0, 16, 0);
    led_strip_set_pixel(led_strip, 1, 0, 16, 0);
    led_strip_set_pixel(led_strip, 5, 0, 16, 0);
    led_strip_set_pixel(led_strip, 6, 0, 16, 0);
    led_strip_refresh(led_strip);
}

void led_right(void)
{
    // int blue = 16;
    // int anzahl =  sizeof(lights)/sizeof(lights[0]);  //25; //sizeof(lights)/sizeof(lights[0]);
    for (int i = 0; i < 25; i++){
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    led_strip_set_pixel(led_strip, 3, 0, 16, 0);
    led_strip_set_pixel(led_strip, 4, 0, 16, 0);
    led_strip_set_pixel(led_strip, 8, 0, 16, 0);
    led_strip_set_pixel(led_strip, 9, 0, 16, 0);
    led_strip_refresh(led_strip);
}


void led_forward(void)
{
    // int blue = 16;
    // int anzahl =  sizeof(lights)/sizeof(lights[0]);  //25; //sizeof(lights)/sizeof(lights[0]);
    for (int i = 0; i < 25; i++){
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    led_strip_set_pixel(led_strip, 21, 0, 16, 0);
    led_strip_set_pixel(led_strip, 22, 0, 16, 0);
    led_strip_set_pixel(led_strip, 23, 0, 16, 0);
    led_strip_set_pixel(led_strip, 24, 0, 16, 0);
    led_strip_set_pixel(led_strip, 20, 0, 16, 0);
    led_strip_set_pixel(led_strip, 21, 0, 16, 0);
    led_strip_set_pixel(led_strip, 22, 0, 16, 0);
    led_strip_set_pixel(led_strip, 23, 0, 16, 0);
    led_strip_set_pixel(led_strip, 24, 0, 16, 0);
    led_strip_refresh(led_strip);
}


void led_backward(void)
{
    // int blue = 16;
    // int anzahl =  sizeof(lights)/sizeof(lights[0]);  //25; //sizeof(lights)/sizeof(lights[0]);
    for (int i = 0; i < 25; i++){
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    led_strip_set_pixel(led_strip, 20, 0, 0, 20);
    led_strip_set_pixel(led_strip, 21, 0, 0, 20);
    led_strip_set_pixel(led_strip, 22, 0, 0, 20);
    led_strip_set_pixel(led_strip, 23, 0, 0, 20);
    led_strip_set_pixel(led_strip, 24, 0, 0, 20);
    led_strip_refresh(led_strip);
}


void led_off(void)
{
    // int blue = 16;
    // int anzahl =  sizeof(lights)/sizeof(lights[0]);  //25; //sizeof(lights)/sizeof(lights[0]);
    for (int i = 0; i < 25; i++){
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }
    led_strip_refresh(led_strip);

}

void led_break(void)
{
    // int blue = 16;
    // int anzahl =  sizeof(lights)/sizeof(lights[0]);  //25; //sizeof(lights)/sizeof(lights[0]);
    for (int i = 0; i < 25; i++){
        led_strip_set_pixel(led_strip, i, 16, 16, 16);
    }
    led_strip_refresh(led_strip);
}


void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 25, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

// static void blink_led(void)
// {
//     /* Set the GPIO level according to the state (LOW or HIGH)*/
//     gpio_set_level(BLINK_GPIO, s_led_state);
// }

// static void configure_led(void)
// {
//     ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
//     gpio_reset_pin(BLINK_GPIO);
//     /* Set the GPIO as a push/pull output */
//     gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
// }


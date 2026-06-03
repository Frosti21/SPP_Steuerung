#include "sensors.h"
#include "motor.h"
#include "adc_shared.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

/* ============================================================
 * PIN DEFINITIONEN
 * ============================================================ */

#define PIN_REGEN       23
#define PIN_TASTER_AUF  33
#define PIN_TASTER_AB   32
#define PIN_END_OBEN    19
#define PIN_END_UNTEN   18

/* ============================================================
 * WIND / ADC KONFIGURATION
 * ============================================================ */

#define WIND_ADC_CHANNEL    ADC_CHANNEL_6   // GPIO34
#define WIND_AVG_SAMPLES    20

#define WIND_MV_MIN         0               // mV bei 0 km/h   → anpassen!
#define WIND_MV_MAX         3300            // mV bei max km/h  → anpassen!
#define WIND_KMH_MAX        150.0f          // km/h bei WIND_MV_MAX → anpassen!

/* ============================================================
 * INTERNE VARIABLEN
 * ============================================================ */

static adc_oneshot_unit_handle_t wind_adc_handle;
static adc_cali_handle_t         wind_cali_handle;

static float wind_samples[WIND_AVG_SAMPLES] = {0};
static int   wind_sample_idx = 0;
static float wind_speed_kmh  = 0.0f;
static float wind_avg_kmh    = 0.0f;

/* ============================================================
 * HILFSFUNKTION: mV → km/h
 * ============================================================ */

static float mv_to_kmh(int mv)
{
    if (mv <= WIND_MV_MIN) return 0.0f;
    if (mv >= WIND_MV_MAX) return WIND_KMH_MAX;

    return ((float)(mv - WIND_MV_MIN) /
            (float)(WIND_MV_MAX - WIND_MV_MIN)) * WIND_KMH_MAX;
}

/* ============================================================
 * INITIALISIERUNG
 * ============================================================ */

void sensors_init(void)
{

        // KEIN adc_oneshot_new_unit mehr!
        wind_adc_handle = motor_adc_handle;  // Handle von motor.c übernehmen

        // Kanal für Windsensor hinzufügen
        adc_oneshot_chan_cfg_t chan_cfg = {
            .atten    = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };
        ESP_ERROR_CHECK(adc_oneshot_config_channel(
            wind_adc_handle, WIND_ADC_CHANNEL, &chan_cfg));

            
    // /* ---- ADC Unit ---- */
    // adc_oneshot_unit_init_cfg_t unit_cfg = {
    //     .unit_id  = ADC_UNIT_1,
    //     .ulp_mode = ADC_ULP_MODE_DISABLE,
    // };
    // ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &wind_adc_handle));

    // /* ---- Kanal ---- */
    // adc_oneshot_chan_cfg_t chan_cfg = {
    //     .atten    = ADC_ATTEN_DB_12,
    //     .bitwidth = ADC_BITWIDTH_12,
    // // };
    // ESP_ERROR_CHECK(adc_oneshot_config_channel(
    //     wind_adc_handle, WIND_ADC_CHANNEL, &chan_cfg));

    /* ---- Kalibrierung ---- */
    adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id  = ADC_UNIT_1,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(
        &cali_cfg, &wind_cali_handle));

    /* ---- Digitale Eingänge ---- */
    gpio_config_t input_conf = {
        .pin_bit_mask = (1ULL << PIN_REGEN)      |
                        (1ULL << PIN_TASTER_AUF) |
                        (1ULL << PIN_TASTER_AB)  |
                        (1ULL << PIN_END_OBEN)   |
                        (1ULL << PIN_END_UNTEN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&input_conf);
}

/* ============================================================
 * WIND UPDATE – alle 50ms vom sensor_monitor_task aufgerufen
 * ============================================================ */

void wind_sensor_update(void)
{
    int raw = 0, mv = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(wind_adc_handle, WIND_ADC_CHANNEL, &raw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(wind_cali_handle, raw, &mv));

    wind_speed_kmh = mv_to_kmh(mv);

    wind_samples[wind_sample_idx] = wind_speed_kmh;
    wind_sample_idx = (wind_sample_idx + 1) % WIND_AVG_SAMPLES;

    float sum = 0.0f;
    for (int i = 0; i < WIND_AVG_SAMPLES; i++) sum += wind_samples[i];
    wind_avg_kmh = sum / WIND_AVG_SAMPLES;
}

/* ============================================================
 * GETTER
 * ============================================================ */

float wind_get_speed(void)   { return wind_speed_kmh; }
float wind_get_average(void) { return wind_avg_kmh;   }

bool rain_is_active(void)            { return gpio_get_level(PIN_REGEN)      == 0; }
bool button_up_is_pressed(void)      { return gpio_get_level(PIN_TASTER_AUF) == 0; }
bool button_down_is_pressed(void)    { return gpio_get_level(PIN_TASTER_AB)  == 0; }
bool end_switch_top_triggered(void)  { return gpio_get_level(PIN_END_OBEN)   == 1; }
bool end_switch_bottom_triggered(void) { return gpio_get_level(PIN_END_UNTEN) == 1; }
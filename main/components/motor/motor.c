#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"

#include "motor.h"
#include <stdio.h>
#include <math.h>

static const char *TAG = "MOTOR";

// ─────────────────────────────────────────────
// Pinbelegung Motoren (BTS7960)
// ─────────────────────────────────────────────
#define MOTOR_A_RPWM_PIN    14
#define MOTOR_A_LPWM_PIN    27
#define MOTOR_A_R_EN_PIN    25
#define MOTOR_A_L_EN_PIN    26

#define MOTOR_B_RPWM_PIN    13
#define MOTOR_B_LPWM_PIN    15
#define MOTOR_B_R_EN_PIN    16
#define MOTOR_B_L_EN_PIN    17

// ─────────────────────────────────────────────
// Strommessung (R_IS / L_IS)
// ─────────────────────────────────────────────
// Widerstand gegen GND an den IS-Pins (in Ohm)
#define CURRENT_SENSE_RESISTOR_OHM   1000.0f    // 1 kΩ
// BTS7960 internes Stromverhältnis: 1 : 8500
#define BTS7960_CURRENT_RATIO        8500.0f
// ADC Referenzspannung ESP32
#define ADC_VREF_MV                  3300.0f
// ADC Auflösung 12-bit
#define ADC_MAX_RAW                  4095.0f
// Stromschwelle für Überlastschutz (Ampere) — noch nicht aktiv
#define MOTOR_OVERCURRENT_THRESHOLD  10.0f

// ADC-Kanäle für IS-Pins (anpassen je nach Verdrahtung)
#define MOTOR_A_IS_ADC_CHANNEL       ADC_CHANNEL_6   // z.B. GPIO34
#define MOTOR_B_IS_ADC_CHANNEL       ADC_CHANNEL_7   // z.B. GPIO35

// ADC Handle (für esp_adc/adc_oneshot API)
static adc_oneshot_unit_handle_t adc_handle = NULL;

// ─────────────────────────────────────────────
// PWM-Konfiguration
// ─────────────────────────────────────────────
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_FREQ_HZ        20000
#define LEDC_DUTY_RES       LEDC_TIMER_10_BIT
#define LEDC_MAX_DUTY       ((1 << LEDC_DUTY_RES) - 1)  // 1023

#define LEDC_CHANNEL_A_R    LEDC_CHANNEL_0   // Motor A Rechtslauf
#define LEDC_CHANNEL_A_L    LEDC_CHANNEL_1   // Motor A Linkslauf
#define LEDC_CHANNEL_B_R    LEDC_CHANNEL_2   // Motor B Rechtslauf
#define LEDC_CHANNEL_B_L    LEDC_CHANNEL_3   // Motor B Linkslauf


// ═════════════════════════════════════════════
// INITIALISIERUNG
// ═════════════════════════════════════════════

void motor_init(void)
{
    // --- LEDC Timer ---
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_MODE,
        .timer_num       = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz         = LEDC_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // --- 4 PWM-Kanäle ---
    gpio_num_t pwm_pins[] = {
        MOTOR_A_RPWM_PIN, MOTOR_A_LPWM_PIN,
        MOTOR_B_RPWM_PIN, MOTOR_B_LPWM_PIN
    };
    ledc_channel_t channels[] = {
        LEDC_CHANNEL_A_R, LEDC_CHANNEL_A_L,
        LEDC_CHANNEL_B_R, LEDC_CHANNEL_B_L
    };
    for (int i = 0; i < 4; i++) {
        ledc_channel_config_t ch = {
            .gpio_num   = pwm_pins[i],
            .speed_mode = LEDC_MODE,
            .channel    = channels[i],
            .timer_sel  = LEDC_TIMER,
            .duty       = 0,
            .hpoint     = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ch));
    }

    // --- Enable-Pins ---
    gpio_config_t io_conf = {
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << MOTOR_A_R_EN_PIN) |
                        (1ULL << MOTOR_A_L_EN_PIN) |
                        (1ULL << MOTOR_B_R_EN_PIN) |
                        (1ULL << MOTOR_B_L_EN_PIN)
    };
    gpio_config(&io_conf);

    gpio_set_level(MOTOR_A_R_EN_PIN, 1);
    gpio_set_level(MOTOR_A_L_EN_PIN, 1);
    gpio_set_level(MOTOR_B_R_EN_PIN, 1);
    gpio_set_level(MOTOR_B_L_EN_PIN, 1);

    // --- ADC für Strommessung initialisieren ---
    adc_oneshot_unit_init_cfg_t adc_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten    = ADC_ATTEN_DB_12   // bis ~3.3V messbar
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, MOTOR_A_IS_ADC_CHANNEL, &chan_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, MOTOR_B_IS_ADC_CHANNEL, &chan_cfg));

    ESP_LOGI(TAG, "Motor-Treiber initialisiert");
}


// ═════════════════════════════════════════════
// MOTOR A
// ═════════════════════════════════════════════

// speed: -1023 (volle Kraft links) bis +1023 (volle Kraft rechts)
void motor_A_set(int speed)
{
    int duty = abs(speed);
    if (speed > 0) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_R, duty);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_L, 0);
    } else if (speed < 0) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_R, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_L, duty);
    } else {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_R, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_L, 0);
    }
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A_R);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A_L);
}

// Aktive Bremse
void motor_A_stop(void)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_R, LEDC_MAX_DUTY);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_L, LEDC_MAX_DUTY);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A_R);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A_L);
}

// Freilauf
void motor_A_coast(void)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_R, 0);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_A_L, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A_R);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_A_L);
}


// ═════════════════════════════════════════════
// MOTOR B
// ═════════════════════════════════════════════

void motor_B_set(int speed)
{
    int duty = abs(speed);
    if (speed > 0) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_R, duty);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_L, 0);
    } else if (speed < 0) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_R, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_L, duty);
    } else {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_R, 0);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_L, 0);
    }
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B_R);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B_L);
}

void motor_B_stop(void)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_R, LEDC_MAX_DUTY);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_L, LEDC_MAX_DUTY);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B_R);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B_L);
}

void motor_B_coast(void)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_R, 0);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B_L, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B_R);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B_L);
}


// ═════════════════════════════════════════════
// STROMMESSUNG (noch nicht aktiv verwendet)
// ═════════════════════════════════════════════
//
// Schaltung: IS-Pin ──── 1kΩ ──── GND
//                            │
//                         ADC-Pin
//
// Formel: I_Motor [A] = (U_IS [V] / R_sense [Ω]) × 8500
//
// Rückgabe: Motorstrom in Ampere, oder -1.0f bei Fehler

float motor_A_get_current(void)
{
    if (adc_handle == NULL) {
        ESP_LOGW(TAG, "ADC nicht initialisiert");
        return -1.0f;
    }

    int adc_raw = 0;
    esp_err_t err = adc_oneshot_read(adc_handle, MOTOR_A_IS_ADC_CHANNEL, &adc_raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC Lesefehler Motor A: %s", esp_err_to_name(err));
        return -1.0f;
    }

    float u_is  = (adc_raw / ADC_MAX_RAW) * ADC_VREF_MV / 1000.0f;  // Volt
    float current = (u_is / CURRENT_SENSE_RESISTOR_OHM) * BTS7960_CURRENT_RATIO;
    return current;
}

float motor_B_get_current(void)
{
    if (adc_handle == NULL) {
        ESP_LOGW(TAG, "ADC nicht initialisiert");
        return -1.0f;
    }

    int adc_raw = 0;
    esp_err_t err = adc_oneshot_read(adc_handle, MOTOR_B_IS_ADC_CHANNEL, &adc_raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC Lesefehler Motor B: %s", esp_err_to_name(err));
        return -1.0f;
    }

    float u_is  = (adc_raw / ADC_MAX_RAW) * ADC_VREF_MV / 1000.0f;
    float current = (u_is / CURRENT_SENSE_RESISTOR_OHM) * BTS7960_CURRENT_RATIO;
    return current;
}

// Beispiel wie man die Strommessung später einsetzen kann:
//
//   float strom = motor_A_get_current();
//   ESP_LOGI(TAG, "Motor A Strom: %.2f A", strom);
//
//   if (strom > MOTOR_OVERCURRENT_THRESHOLD) {
//       motor_A_stop();   // Endlage erkannt oder Überlast
//   }
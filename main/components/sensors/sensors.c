#include "sensors.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"

/* ============================================================
 * PIN DEFINITIONEN
 * ============================================================ */

#define PIN_WIND        16      // Windsensor (Reedkontakt)
#define PIN_REGEN       23      // Regensensor (LOW = aktiv)
#define PIN_TASTER_AUF  32      // Taster Auf  (LOW = gedrückt)
#define PIN_TASTER_AB   33      // Taster Ab   (LOW = gedrückt)
#define PIN_END_OBEN    18      // Endschalter oben  (NC: LOW = ausgelöst)
#define PIN_END_UNTEN   19      // Endschalter unten (NC: LOW = ausgelöst)

/* ============================================================
 * WIND KONFIGURATION
 * ============================================================ */

#define WIND_AVG_COUNT          5       // Anzahl Werte für gleitenden Durchschnitt
#define WIND_CALC_INTERVAL_MS   1000    // Berechnungsintervall in ms
#define WIND_DEBOUNCE_US        10000   // Entprellzeit in Mikrosekunden (10ms)
#define PULSES_PER_SECOND_TO_KMH 2.0f  // Umrechnungsfaktor Impulse/s → km/h

/* ============================================================
 * INTERNE VARIABLEN - WIND
 * ============================================================ */

static volatile uint32_t  wind_pulses       = 0;
static volatile int64_t   wind_last_pulse   = 0;

static float wind_speed                     = 0.0f;
static float wind_average                   = 0.0f;
static float wind_buffer[WIND_AVG_COUNT]    = {0};
static int   wind_index                     = 0;

/* ============================================================
 * ISR - WINDSENSOR
 * ============================================================ */

static void IRAM_ATTR wind_isr_handler(void *arg)
{
    int64_t now = esp_timer_get_time();   // Mikrosekunden

    if ((now - wind_last_pulse) > WIND_DEBOUNCE_US) {
        wind_pulses++;
        wind_last_pulse = now;
    }
}

/* ============================================================
 * TASK - WINDBERECHNUNG
 * Läuft alle WIND_CALC_INTERVAL_MS und berechnet
 * Geschwindigkeit + gleitenden Durchschnitt
 * ============================================================ */

static void wind_calc_task(void *arg)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(WIND_CALC_INTERVAL_MS));

        /* Kritischer Abschnitt: Zähler atomar lesen + zurücksetzen */
        portDISABLE_INTERRUPTS();
        uint32_t pulses = wind_pulses;
        wind_pulses = 0;
        portENABLE_INTERRUPTS();

        /* Geschwindigkeit berechnen */
        wind_speed = (float)pulses * PULSES_PER_SECOND_TO_KMH;

        /* Ringpuffer befüllen */
        wind_buffer[wind_index] = wind_speed;
        wind_index = (wind_index + 1) % WIND_AVG_COUNT;

        /* Gleitenden Durchschnitt berechnen */
        float sum = 0.0f;
        for (int i = 0; i < WIND_AVG_COUNT; i++) {
            sum += wind_buffer[i];
        }
        wind_average = sum / WIND_AVG_COUNT;
    }
}

/* ============================================================
 * INITIALISIERUNG
 * ============================================================ */

void sensors_init(void)
{
    /* ---- Windsensor (Interrupt, fallende Flanke) ---- */
    gpio_config_t wind_conf = {
        .pin_bit_mask = (1ULL << PIN_WIND),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&wind_conf);

    /* ---- Eingänge ohne Interrupt (kein intr_type nötig) ---- */
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

    /* ---- ISR Service + Handler registrieren ---- */
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_WIND, wind_isr_handler, NULL);

    /* ---- Wind-Berechnungs-Task starten ---- */
    xTaskCreate(
        wind_calc_task,     // Task-Funktion
        "wind_calc",        // Name (Debug)
        2048,               // Stack in Bytes
        NULL,               // Parameter
        5,                  // Priorität
        NULL                // Task-Handle (nicht benötigt)
    );
}

/* ============================================================
 * ÖFFENTLICHE GETTER - WIND
 * ============================================================ */

float wind_get_speed(void)
{
    return wind_speed;
}

float wind_get_average(void)
{
    return wind_average;
}

/* ============================================================
 * ÖFFENTLICHE GETTER - SENSOREN
 * ============================================================ */

bool rain_is_active(void)
{
    return gpio_get_level(PIN_REGEN) == 0;
}

bool button_up_is_pressed(void)
{
    return gpio_get_level(PIN_TASTER_AUF) == 0;
}

bool button_down_is_pressed(void)
{
    return gpio_get_level(PIN_TASTER_AB) == 0;
}

/* NC-Endschalter: im Normalzustand geschlossen (HIGH durch Pull-up)
   Ausgelöst = Kontakt öffnet = Pin geht auf LOW */
bool end_switch_top_triggered(void)
{
    return gpio_get_level(PIN_END_OBEN) == 0;
}

bool end_switch_bottom_triggered(void)
{
    return gpio_get_level(PIN_END_UNTEN) == 0;
}
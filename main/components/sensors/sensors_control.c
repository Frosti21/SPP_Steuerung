
#include "sensors_control.h"
#include "sensors.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define TAG "SENSOR_CONTROL"

/* ============================================================
 * KONFIGURATION
 * ============================================================ */

#define SENSOR_POLL_MS      50      // Pollingrate für Zustandserkennung
#define SENSOR_QUEUE_SIZE   10      // Max. Events in der Queue

/* Windgeschwindigkeit: ab welcher Änderung ein Event gesendet wird */
#define WIND_CHANGE_THRESHOLD  0.5f

/* ============================================================
 * ÖFFENTLICHE QUEUE
 * ============================================================ */

QueueHandle_t sensor_queue = NULL;

/* ============================================================
 * INTERNER ZUSTAND (letzter bekannter Zustand je Sensor)
 * ============================================================ */

typedef struct {
    bool  regen;
    bool  taster_auf;
    bool  taster_ab;
    bool  end_oben;
    bool  end_unten;
    float wind_avg;
} sensor_state_t;

/* ============================================================
 * HILFSFUNKTION: Event in Queue schicken
 * ============================================================ */

static void send_event(sensor_id_t id, bool state, float speed, float avg)
{
    sensor_event_t event = {
        .sensor     = id,
        .state      = state,
        .wind_speed = speed,
        .wind_avg   = avg,
    };

    if (xQueueSend(sensor_queue, &event, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Queue voll – Event von Sensor %d verworfen", id);
    }
}

/* ============================================================
 * SENSOR MONITOR TASK
 * Pollt alle Sensoren und erkennt Zustandsänderungen
 * ============================================================ */

static void sensor_monitor_task(void *arg)
{
    sensor_state_t last = {
        .regen      = false,
        .taster_auf = false,
        .taster_ab  = false,
        .end_oben   = false,
        .end_unten  = false,
        .wind_avg   = 0.0f,
    };

    ESP_LOGI(TAG, "Monitor Task gestartet");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(SENSOR_POLL_MS));

        /* ---- Regensensor ---- */
        bool regen = rain_is_active();
        if (regen != last.regen) {
            last.regen = regen;
            ESP_LOGI(TAG, "Regen: %s", regen ? "aktiv" : "inaktiv");
            send_event(SENSOR_REGEN, regen, 0.0f, 0.0f);
        }

        /* ---- Taster Auf ---- */
        bool taster_auf = button_up_is_pressed();
        if (taster_auf != last.taster_auf) {
            last.taster_auf = taster_auf;
            ESP_LOGI(TAG, "Taster Auf: %s", taster_auf ? "gedrückt" : "losgelassen");
            send_event(SENSOR_TASTER_AUF, taster_auf, 0.0f, 0.0f);
        }

        /* ---- Taster Ab ---- */
        bool taster_ab = button_down_is_pressed();
        if (taster_ab != last.taster_ab) {
            last.taster_ab = taster_ab;
            ESP_LOGI(TAG, "Taster Ab: %s", taster_ab ? "gedrückt" : "losgelassen");
            send_event(SENSOR_TASTER_AB, taster_ab, 0.0f, 0.0f);
        }

        /* ---- Endschalter Oben ---- */
        bool end_oben = end_switch_top_triggered();
        if (end_oben != last.end_oben) {
            last.end_oben = end_oben;
            ESP_LOGI(TAG, "Endschalter Oben: %s", end_oben ? "ausgelöst" : "frei");
            send_event(SENSOR_END_OBEN, end_oben, 0.0f, 0.0f);
        }

        /* ---- Endschalter Unten ---- */
        bool end_unten = end_switch_bottom_triggered();
        if (end_unten != last.end_unten) {
            last.end_unten = end_unten;
            ESP_LOGI(TAG, "Endschalter Unten: %s", end_unten ? "ausgelöst" : "frei");
            send_event(SENSOR_END_UNTEN, end_unten, 0.0f, 0.0f);
        }

        /* ---- Windsensor (nur bei relevanter Änderung) ---- */
        float avg = wind_get_average();
        if ((avg - last.wind_avg) >  WIND_CHANGE_THRESHOLD ||
            (avg - last.wind_avg) < -WIND_CHANGE_THRESHOLD) {
            last.wind_avg = avg;
            float speed = wind_get_speed();
            ESP_LOGI(TAG, "Wind: %.1f km/h (Ø %.1f km/h)", speed, avg);
            send_event(SENSOR_WIND, true, speed, avg);
        }
    }
}

/* ============================================================
 * INITIALISIERUNG
 * ============================================================ */

void sensor_control_init(void)
{
    ESP_LOGI(TAG, "Start Init");

    sensor_queue = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(sensor_event_t));
    if (sensor_queue == NULL) {
        ESP_LOGE(TAG, "Queue konnte nicht erstellt werden!");
        return;
    }

    sensors_init();

    xTaskCreate(
        sensor_monitor_task,    // Task-Funktion
        "SENSOR_MONITOR",       // Name (Debug)
        2048,                   // Stack in Bytes
        NULL,                   // Parameter
        6,                      // Priorität
        NULL                    // Task-Handle (nicht benötigt)
    );

    ESP_LOGI(TAG, "Ende Init");
}
#ifndef SENSORS_H
#define SENSORS_H

#include <stdbool.h>
#include <stdint.h>

/* ============================================================
 * SENSORS.H
 * Öffentliche Schnittstelle für alle Sensoren
 * - Windsensor (Reedkontakt)
 * - Regensensor
 * - Taster Auf / Ab
 * - Endschalter Oben / Unten (NC - Sicherheitsprinzip)
 * ============================================================ */

/* ------------------------------------------------------------
 * Initialisierung
 * ------------------------------------------------------------ */

/**
 * @brief Alle Sensoren initialisieren (GPIOs + ISR)
 *        Einmalig in app_main() aufrufen.
 */
void sensors_init(void);

/* ------------------------------------------------------------
 * Windsensor
 * ------------------------------------------------------------ */

/**
 * @brief Aktuelle Windgeschwindigkeit in km/h
 *        Wird jede Sekunde intern aktualisiert.
 */
float wind_get_speed(void);

/**
 * @brief Gleitender Durchschnitt der Windgeschwindigkeit (km/h)
 *        Über die letzten WIND_AVG_COUNT Messungen.
 */
float wind_get_average(void);

/* ------------------------------------------------------------
 * Regensensor
 * ------------------------------------------------------------ */

/**
 * @brief RegenStatus lesen
 * @return true  = Regen erkannt
 *         false = kein Regen
 */
bool rain_is_active(void);

/* ------------------------------------------------------------
 * Taster
 * ------------------------------------------------------------ */

/**
 * @brief Taster "Auf" Status
 * @return true = Taster gedrückt
 */
bool button_up_is_pressed(void);

/**
 * @brief Taster "Ab" Status
 * @return true = Taster gedrückt
 */
bool button_down_is_pressed(void);

/* ------------------------------------------------------------
 * Endschalter (Rollenschalter, NC)
 * ------------------------------------------------------------ */

/**
 * @brief Endschalter oben ausgelöst?
 * @return true = Endlage oben erreicht
 */
bool end_switch_top_triggered(void);

/**
 * @brief Endschalter unten ausgelöst?
 * @return true = Endlage unten erreicht
 */
bool end_switch_bottom_triggered(void);
#endif // SENSORS_H

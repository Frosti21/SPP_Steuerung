#ifndef ADC_SHARED_H
#define ADC_SHARED_H

#include "esp_adc/adc_oneshot.h"

// Einmalig initialisiert in adc_shared.c
// Von allen Komponenten genutzt
extern adc_oneshot_unit_handle_t adc1_shared_handle;

void adc_shared_init(void);

#endif // ADC_SHARED_H
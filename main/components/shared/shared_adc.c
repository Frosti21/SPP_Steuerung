#include "shared_adc.h"

adc_oneshot_unit_handle_t adc1_shared_handle = NULL;

void adc_shared_init(void)
{
    if (adc1_shared_handle != NULL) return;  // Schutz gegen Doppelaufruf

    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id  = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc1_shared_handle));
}
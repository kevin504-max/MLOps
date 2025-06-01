#ifndef MQ4_SENSOR_H
#define MQ4_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_adc/adc_oneshot.h"

/**
 * @brief Inicializa o canal ADC para leitura do sensor MQ-4.
 */
void mq4_sensor_init(adc_oneshot_unit_handle_t handle);

/**
 * @brief Inicia a task de leitura periódica do sensor MQ-4.
 */
void mq4_start_read_task(void);

#ifdef __cplusplus
}
#endif

#endif
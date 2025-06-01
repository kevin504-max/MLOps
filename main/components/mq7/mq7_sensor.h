#ifndef MQ7_SENSOR_H
#define MQ7_SENSOR_H

#include "esp_adc/adc_oneshot.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa o canal ADC usado pelo sensor MQ-7.
 * 
 * @param handle Handle do ADC one-shot (deve ser previamente inicializado).
 */
void mq7_sensor_init(adc_oneshot_unit_handle_t handle);

/**
 * @brief Inicia a task que realiza a leitura periódica do sensor MQ-7.
 */
void mq7_start_read_task(void);

#ifdef __cplusplus
}
#endif

#endif
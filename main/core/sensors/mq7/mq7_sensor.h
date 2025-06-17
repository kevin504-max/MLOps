#ifndef MQ7_SENSOR_H
#define MQ7_SENSOR_H

#include "esp_adc/adc_oneshot.h"

/**
 * @brief Inicializa o sensor MQ-7 configurando o ADC.
 *
 * @param handle Handle do ADC oneshot.
 */
void mq7_sensor_init(adc_oneshot_unit_handle_t handle);

/**
 * @brief Inicia a tarefa FreeRTOS que realiza a leitura periódica do sensor MQ-7.
 */
void mq7_start_read_task(void);

#endif // MQ7_SENSOR_H
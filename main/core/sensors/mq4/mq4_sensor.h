#ifndef MQ4_SENSOR_H
#define MQ4_SENSOR_H

#include "esp_adc/adc_oneshot.h"

/**
 * @brief Inicializa o sensor MQ-4 configurando o ADC.
 *
 * @param handle Handle do ADC oneshot.
 */
void mq4_sensor_init(adc_oneshot_unit_handle_t handle);

/**
 * @brief Inicia a tarefa FreeRTOS que lê periodicamente o sensor MQ-4.
 */
void mq4_start_read_task(void);

#endif // MQ4_SENSOR_H
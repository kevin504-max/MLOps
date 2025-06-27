/**
    * @file mq7_sensor.h
    * @brief Header file for MQ-7 sensor initialization and reading task.
    *
    * This module provides functions to initialize the MQ-7 gas sensor and to create a
    * FreeRTOS task that periodically reads data from the sensor using ADC.
*/

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
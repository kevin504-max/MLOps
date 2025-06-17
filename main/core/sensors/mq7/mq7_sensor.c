/**
 * @file mq7_sensor.c
 * @brief Driver para leitura do sensor de monóxido de carbono MQ-7 usando ADC do ESP32.
 *
 * Este módulo configura o ADC para o canal do sensor MQ-7,
 * cria uma tarefa FreeRTOS para leitura periódica do valor analógico,
 * converte a leitura em tensão e estima o valor em PPM (partes por milhão).
 */

#include "mq7_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#define MQ7_LOG_TAG "MQ7_SENSOR"
#define MQ7_ADC_CHANNEL ADC_CHANNEL_7  // GPIO35 no ESP32
#define MQ7_VREF 3.3f
#define MQ7_RESOLUTION 4095.0f

static adc_oneshot_unit_handle_t adc1_handle_internal;

// Função externa (main.c) para atualizar os dados do sensor
extern void update_mq7_data(float voltage, float ppm);

/**
 * @brief Converte a tensão lida em PPM de monóxido de carbono (CO).
 *
 * Ajuste essa fórmula conforme a calibração real do sensor MQ-7.
 *
 * @param voltage Tensão do sensor em volts.
 * @return Valor estimado em PPM de CO.
 */
static float mq7_voltage_to_ppm(float voltage) {
    return (voltage * 1000.0f) / 4.0f;
}

/**
 * @brief Tarefa FreeRTOS que lê periodicamente o sensor MQ-7.
 *
 * Realiza leitura ADC, converte para tensão e PPM, e atualiza o sistema.
 *
 * @param pvParameters Parâmetro da tarefa (não usado).
 */
static void mq7_read_task(void *pvParameters) {
    while (true) {
        int raw_value = 0;
        esp_err_t ret = adc_oneshot_read(adc1_handle_internal, MQ7_ADC_CHANNEL, &raw_value);

        if (ret != ESP_OK) {
            ESP_LOGE(MQ7_LOG_TAG, "ADC read failed: %s", esp_err_to_name(ret));
            vTaskDelay(pdMS_TO_TICKS(10000));
            continue;
        }

        float voltage = raw_value * (MQ7_VREF / MQ7_RESOLUTION);
        float ppm = mq7_voltage_to_ppm(voltage);

        ESP_LOGI(MQ7_LOG_TAG, "Raw: %d, Voltage: %.2f V, CO PPM: %.2f", raw_value, voltage, ppm);
        update_mq7_data(voltage, ppm);

        vTaskDelay(pdMS_TO_TICKS(10000));  // 10 segundos entre leituras
    }
}

/**
 * @brief Inicializa o sensor MQ-7 configurando o ADC.
 *
 * @param handle Handle do ADC oneshot.
 */
void mq7_sensor_init(adc_oneshot_unit_handle_t handle) {
    adc1_handle_internal = handle;

    adc_oneshot_chan_cfg_t channel_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    esp_err_t ret = adc_oneshot_config_channel(adc1_handle_internal, MQ7_ADC_CHANNEL, &channel_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(MQ7_LOG_TAG, "Failed to configure MQ7 ADC channel: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(MQ7_LOG_TAG, "MQ7 ADC channel configured successfully");
    }
}

/**
 * @brief Inicia a tarefa FreeRTOS que realiza a leitura do sensor MQ-7.
 */
void mq7_start_read_task(void) {
    xTaskCreate(mq7_read_task, "mq7_read_task", 2048, NULL, 5, NULL);
}
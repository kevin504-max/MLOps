/**
 * @file mq4_sensor.c
 * @brief Driver para leitura do sensor de gás MQ-4 usando ADC do ESP32.
 *
 * Este módulo inicializa o ADC, configura o canal para o sensor MQ-4,
 * e cria uma tarefa FreeRTOS para leitura periódica da tensão do sensor,
 * convertendo-a em PPM (partes por milhão) de gás metano (CH4).
 */

#include "mq4_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#define MQ4_LOG_TAG "MQ4_SENSOR"
#define MQ4_ADC_CHANNEL ADC_CHANNEL_6  // GPIO34 no ESP32
#define MQ4_VREF 3.3f
#define MQ4_RESOLUTION 4095.0f

static adc_oneshot_unit_handle_t adc1_handle_internal;

// Função externa para atualizar os dados no sistema principal
extern void update_mq4_data(float voltage, float ppm);

/**
 * @brief Converte a tensão lida do sensor MQ-4 em PPM de gás.
 *
 * Ajustar essa fórmula com base na calibração específica do sensor MQ-4.
 *
 * @param voltage Tensão lida do sensor (em volts).
 * @return PPM estimado de gás metano.
 */
static float mq4_voltage_to_ppm(float voltage) {
    return (voltage * 1000.0f) / 4.0f;
}

/**
 * @brief Tarefa FreeRTOS que lê periodicamente o sensor MQ-4.
 *
 * Realiza leitura ADC, converte para tensão e PPM, e atualiza os dados.
 *
 * @param pvParameters Parâmetro da tarefa (não usado).
 */
static void mq4_read_task(void *pvParameters) {
    while (true) {
        int raw_adc_value = 0;
        esp_err_t result = adc_oneshot_read(adc1_handle_internal, MQ4_ADC_CHANNEL, &raw_adc_value);

        if (result != ESP_OK) {
            ESP_LOGE(MQ4_LOG_TAG, "ADC read failed: %s", esp_err_to_name(result));
            vTaskDelay(pdMS_TO_TICKS(10000));
            continue;
        }

        float voltage = raw_adc_value * (MQ4_VREF / MQ4_RESOLUTION);
        float ppm = mq4_voltage_to_ppm(voltage);

        ESP_LOGI(MQ4_LOG_TAG, "Raw: %d -> Voltage: %.2f V, CH4 PPM: %.2f", raw_adc_value, voltage, ppm);
        update_mq4_data(voltage, ppm);

        vTaskDelay(pdMS_TO_TICKS(10000));  // Delay 10 segundos entre leituras
    }
}

/**
 * @brief Inicializa o sensor MQ-4 configurando o ADC.
 *
 * @param handle Handle do ADC oneshot já criado.
 */
void mq4_sensor_init(adc_oneshot_unit_handle_t handle) {
    adc1_handle_internal = handle;

    adc_oneshot_chan_cfg_t channel_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    esp_err_t ret = adc_oneshot_config_channel(adc1_handle_internal, MQ4_ADC_CHANNEL, &channel_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(MQ4_LOG_TAG, "Failed to configure MQ4 ADC channel: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(MQ4_LOG_TAG, "MQ4 ADC channel configured successfully");
    }
}

/**
 * @brief Inicia a tarefa FreeRTOS de leitura do sensor MQ-4.
 */
void mq4_start_read_task(void) {
    xTaskCreate(mq4_read_task, "mq4_read_task", 2048, NULL, 5, NULL);
}
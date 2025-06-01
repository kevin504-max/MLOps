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

// Função externa (main.c) para enviar dados atualizados
extern void update_mq7_data(float voltage, float ppm);

/**
 * @brief Converte a tensão em PPM de CO.
 *
 * Ajuste essa fórmula conforme a calibração do sensor MQ-7 real.
 */
static float mq7_voltage_to_ppm(float voltage) {
    return (voltage * 1000.0f) / 4.0f;
}

/**
 * @brief Task responsável pela leitura periódica do sensor MQ-7.
 */
static void mq7_read_task(void *pvParameters) {
    while (true) {
        int raw_value = 0;
        esp_err_t ret = adc_oneshot_read(adc1_handle_internal, MQ7_ADC_CHANNEL, &raw_value);

        if (ret != ESP_OK) {
            ESP_LOGE(MQ7_LOG_TAG, "ADC read failed: %s", esp_err_to_name(ret));
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        float voltage = raw_value * (MQ7_VREF / MQ7_RESOLUTION);
        float ppm = mq7_voltage_to_ppm(voltage);

        ESP_LOGI(MQ7_LOG_TAG, "Raw: %d, Voltage: %.2f V, CO PPM: %.2f", raw_value, voltage, ppm);
        update_mq7_data(voltage, ppm);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void mq7_sensor_init(adc_oneshot_unit_handle_t handle) {
    adc1_handle_internal = handle;

    adc_oneshot_chan_cfg_t channel_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle_internal, MQ7_ADC_CHANNEL, &channel_cfg));
}

void mq7_start_read_task(void) {
    xTaskCreate(mq7_read_task, "mq7_read_task", 2048, NULL, 5, NULL);
}
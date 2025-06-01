#include "mq4_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#define MQ4_LOG_TAG "MQ4_SENSOR"
#define MQ4_ADC_CHANNEL ADC_CHANNEL_6  // GPIO34 no ESP32
#define MQ4_VREF 3.3f
#define MQ4_RESOLUTION 4095.0f

// Função externa definida no main.c para receber o valor de tensão
extern void update_mq4_data(float voltage);
static adc_oneshot_unit_handle_t adc1_handle_internal;

/**
 * @brief Task responsável pela leitura periódica do sensor MQ-4
 */
static void mq4_read_task(void *pvParameters) {
    while (true) {
        int raw_adc_value = 0;
        esp_err_t result = adc_oneshot_read(adc1_handle_internal, MQ4_ADC_CHANNEL, &raw_adc_value);

        if (result != ESP_OK) {
            ESP_LOGE(MQ4_LOG_TAG, "ADC read failed: %s", esp_err_to_name(result));
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        float voltage = raw_adc_value * (MQ4_VREF / MQ4_RESOLUTION);
        ESP_LOGI(MQ4_LOG_TAG, "Raw ADC: %d -> Voltage: %.2f V", raw_adc_value, voltage);

        update_mq4_data(voltage);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void mq4_sensor_init(adc_oneshot_unit_handle_t handle) {
    adc1_handle_internal = handle;

    adc_oneshot_chan_cfg_t channel_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle_internal, MQ4_ADC_CHANNEL, &channel_cfg));
}

void mq4_start_read_task(void) {
    xTaskCreate(mq4_read_task, "mq4_read_task", 2048, NULL, 5, NULL);
}
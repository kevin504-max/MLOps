#include "mq7_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include <math.h>

#define MQ7_LOG_TAG "MQ7_SENSOR"
#define MQ7_ADC_CHANNEL ADC_CHANNEL_7  // GPIO35 no ESP32
#define MQ7_VREF 3.3f
#define MQ7_RESOLUTION 4095.0f
#define MQ7_RL 10000.0f
#define MQ7_RO_CLEAN_AIR 27.5f

static adc_oneshot_unit_handle_t adc1_handle_internal;
static float mq7_ro = 10.0f;

extern void update_mq7_data(float voltage, float ppm);

static float calculate_rs(float vout) {
    return (MQ7_VREF - vout) * MQ7_RL / vout;
}

static float mq7_rs_to_ppm(float rs) {
    float ratio = rs / mq7_ro;
    return 99.042f * powf(ratio, -1.518f);
}

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
        float rs = calculate_rs(voltage);
        float ppm = mq7_rs_to_ppm(rs);

        ESP_LOGI(MQ7_LOG_TAG, "Raw: %d, Voltage: %.2f V, Rs: %.2f, CO_PPM: %.2f", raw_value, voltage, rs, ppm);
        update_mq7_data(voltage, ppm);

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

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

void mq7_start_read_task(void) {
    xTaskCreate(mq7_read_task, "mq7_read_task", 2048, NULL, 5, NULL);
}
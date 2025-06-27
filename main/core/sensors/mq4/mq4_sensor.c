#include "mq4_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include <math.h>

#define MQ4_LOG_TAG "MQ4_SENSOR"
#define MQ4_ADC_CHANNEL ADC_CHANNEL_6  // GPIO34 no ESP32
#define MQ4_VREF 3.3f
#define MQ4_RESOLUTION 4095.0f
#define MQ4_RL 10000.0f
#define MQ4_RO_CLEAN_AIR 9.83f  // Rs/Ro para ar limpo (datasheet)

static adc_oneshot_unit_handle_t adc1_handle_internal;
static float mq4_ro = 10.0f; // valor inicial estimado, pode ser calibrado

extern void update_mq4_data(float voltage, float ppm);

static float calculate_rs(float vout) {
    return (MQ4_VREF - vout) * MQ4_RL / vout;
}

static float mq4_rs_to_ppm(float rs) {
    float ratio = rs / mq4_ro;
    return 625.0f * powf(ratio, -2.1f);  // valores da curva CH4 x Rs/Ro
}

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
        float rs = calculate_rs(voltage);
        float ppm = mq4_rs_to_ppm(rs);

        ESP_LOGI(MQ4_LOG_TAG, "Raw: %d, Voltage: %.2f V, Rs: %.2f, CH4_PPM: %.2f", raw_adc_value, voltage, rs, ppm);
        update_mq4_data(voltage, ppm);

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

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

void mq4_start_read_task(void) {
    xTaskCreate(mq4_read_task, "mq4_read_task", 2048, NULL, 5, NULL);
}
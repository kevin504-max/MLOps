/**
 * @file mq4_sensor.c
 * @brief Driver for reading and converting data from the MQ-4 gas sensor.
 *
 * This module provides functionality to read analog voltage from the MQ-4 sensor
 * using the ESP32 ADC, convert the voltage into sensor resistance (Rs), and then
 * estimate methane (CH₄) concentration in parts per million (ppm) based on a calibration ratio (Rs/Ro).
 */

#include "mq4_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include <math.h>

#define MQ4_LOG_TAG "MQ4_SENSOR"
#define MQ4_ADC_CHANNEL ADC_CHANNEL_6  ///< GPIO34 on ESP32
#define MQ4_VREF 3.3f                  ///< Reference voltage for ADC conversion
#define MQ4_RESOLUTION 4095.0f        ///< 12-bit ADC resolution
#define MQ4_RL 10000.0f               ///< Load resistance in ohms
#define MQ4_RO_CLEAN_AIR 9.83f        ///< Rs/Ro ratio in clean air from datasheet

static adc_oneshot_unit_handle_t adc1_handle_internal;
static float mq4_ro = 10.0f;  ///< Initial Ro value, can be calibrated

extern void update_mq4_data(float voltage, float ppm);

/**
 * @brief Calculates sensor resistance (Rs) from output voltage.
 *
 * @param vout Measured output voltage from the MQ-4 sensor.
 * @return Calculated Rs value.
 */
static float calculate_rs(float vout) {
    return (MQ4_VREF - vout) * MQ4_RL / vout;
}

/**
 * @brief Converts Rs to methane concentration (ppm) using the MQ-4 logarithmic curve.
 *
 * Uses the datasheet's CH₄ characteristic curve to convert the Rs/Ro ratio to ppm.
 *
 * @param rs Sensor resistance value.
 * @return Estimated CH₄ concentration in ppm.
 */
static float mq4_rs_to_ppm(float rs) {
    float ratio = rs / mq4_ro;
    return 625.0f * powf(ratio, -2.1f);
}

/**
 * @brief FreeRTOS task that periodically reads data from the MQ-4 sensor.
 *
 * This task reads the ADC value, converts it to voltage and then to Rs,
 * estimates the gas concentration in ppm, logs the result, and updates the shared state.
 *
 * Runs indefinitely with a 10-second delay between readings.
 *
 * @param pvParameters Unused parameter.
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
        float rs = calculate_rs(voltage);
        float ppm = mq4_rs_to_ppm(rs);

        ESP_LOGI(MQ4_LOG_TAG, "Raw: %d, Voltage: %.2f V, Rs: %.2f, CH4_PPM: %.2f", raw_adc_value, voltage, rs, ppm);
        update_mq4_data(voltage, ppm);

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief Initializes the MQ-4 ADC channel configuration.
 *
 * This function stores the ADC handle and configures the channel parameters.
 * Logs success or failure during initialization.
 *
 * @param handle Initialized ADC oneshot unit handle.
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
 * @brief Starts the FreeRTOS task to read MQ-4 sensor data.
 *
 * Creates the `mq4_read_task` with a moderate priority to run periodically in the background.
 */
void mq4_start_read_task(void) {
    xTaskCreate(mq4_read_task, "mq4_read_task", 2048, NULL, 5, NULL);
}

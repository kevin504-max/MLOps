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
#define MQ4_VREF 3.28f                  ///< Reference voltage for ADC conversion
#define MQ4_RESOLUTION 4095.0f        ///< 12-bit ADC resolution
#define MQ4_RL 10000.0f               ///< Load resistance in ohms
#define MQ4_RO_CLEAN_AIR 9.83f        ///< Rs/Ro ratio in clean air from datasheet
#define MQ4_AUTO_CALIBRATE_ON_START 1  ///< auto calibrate Ro on startup


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
 * @brief Calibrates the MQ-4 sensor by measuring Ro in clean air.
 *
 * Reads the ADC value, converts it to voltage, calculates Rs, and sets Ro.
 * Logs the calibration results.
 */
void mq4_calibrate(void) {
    int raw;
    if (adc_oneshot_read(adc1_handle_internal, MQ4_ADC_CHANNEL, &raw) != ESP_OK) {
        ESP_LOGE(MQ4_LOG_TAG, "ADC read failed during calibration");
        return;
    }
    float voltage = raw * (MQ4_VREF / MQ4_RESOLUTION);
    float rs = calculate_rs(voltage);
    mq4_ro = rs / MQ4_RO_CLEAN_AIR;
    ESP_LOGI(MQ4_LOG_TAG, "Calibration complete: Ro = %.2f (Raw: %d, V: %.2f, Rs: %.2f)", mq4_ro, raw, voltage, rs);
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
    float last_valid_voltage = 0.0f;
    float last_valid_ppm = 0.0f;
    bool first_valid_reading = true;

    const float MIN_VOLTAGE = 0.1f;
    const float MAX_VOLTAGE = MQ4_VREF;
    const float MIN_PPM = 0.0f;
    const float MAX_PPM = 10000.0f;
    const float MAX_VOLTAGE_CHANGE = 0.5f;

    const float MAX_PPM_CHANGE = 500.0f;

    while (true) {
        int raw_adc_value = 0;
        if (adc_oneshot_read(adc1_handle_internal, MQ4_ADC_CHANNEL, &raw_adc_value) != ESP_OK) {
            ESP_LOGE(MQ4_LOG_TAG, "ADC read failed");
            vTaskDelay(pdMS_TO_TICKS(10000));
            continue;
        }

        float voltage = raw_adc_value * (MQ4_VREF / MQ4_RESOLUTION);
        float rs = calculate_rs(voltage);
        float ppm = mq4_rs_to_ppm(rs);

        bool data_valid = true;

        if (voltage < MIN_VOLTAGE || voltage > MAX_VOLTAGE) {
            ESP_LOGE(MQ4_LOG_TAG, "Invalid Tension: %.2f V (Raw: %d)", voltage, raw_adc_value);
            data_valid = false;
        }

        if (ppm < MIN_PPM || ppm > MAX_PPM) {
            ESP_LOGE(MQ4_LOG_TAG, "Invalid Concentration: %.2f ppm", ppm);
            data_valid = false;
        }

        if (data_valid && !first_valid_reading) {
            float voltage_change = fabs(voltage - last_valid_voltage);
            float ppm_change = fabs(ppm - last_valid_ppm);

            if (voltage_change > MAX_VOLTAGE_CHANGE) {
                ESP_LOGE(MQ4_LOG_TAG, "Tension variation too high: Δ%.2f V", voltage_change);
                data_valid = false;
            }

            if (ppm_change > MAX_PPM_CHANGE) {
                ESP_LOGE(MQ4_LOG_TAG, "Concentration variation too high: Δ%.2f ppm", ppm_change);
                data_valid = false;
            }
        }

        if (data_valid) {
            ESP_LOGI(MQ4_LOG_TAG, "Raw: %d, Voltage: %.2f V, Rs: %.2f, CH4_PPM: %.2f", 
                    raw_adc_value, voltage, rs, ppm);
            update_mq4_data(voltage, ppm);

            last_valid_voltage = voltage;
            last_valid_ppm = ppm;
            first_valid_reading = false;
        } else {
            ESP_LOGW(MQ4_LOG_TAG, "Discarted Data - Values: Raw=%d, V=%.2f, PPM=%.2f", 
                    raw_adc_value, voltage, ppm);
        }

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

    if (adc_oneshot_config_channel(adc1_handle_internal, MQ4_ADC_CHANNEL, &channel_cfg) == ESP_OK) {
        ESP_LOGI(MQ4_LOG_TAG, "MQ4 ADC channel configured successfully");

    #if MQ4_AUTO_CALIBRATE_ON_START
        mq4_calibrate();
    #endif

    } else {
        ESP_LOGE(MQ4_LOG_TAG, "Failed to configure MQ4 ADC channel");
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

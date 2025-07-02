/**
 * @file mq7_sensor.c
 * @brief Driver for reading and converting data from the MQ-7 gas sensor.
 *
 * This module is responsible for interfacing with the MQ-7 carbon monoxide (CO) sensor,
 * reading analog voltage through the ESP32 ADC, computing sensor resistance (Rs),
 * and estimating CO concentration in parts per million (ppm) using a calibration model.
 */

#include "mq7_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include <math.h>

#define MQ7_LOG_TAG "MQ7_SENSOR"
#define MQ7_ADC_CHANNEL ADC_CHANNEL_7     ///< GPIO35 on ESP32
#define MQ7_VREF 5.0f                      ///< Reference voltage for ADC
#define MQ7_RESOLUTION 4095.0f            ///< ADC 12-bit resolution
#define MQ7_RL 10000.0f                   ///< Load resistance (Ohms)
#define MQ7_RO_CLEAN_AIR 27.5f            ///< Rs/Ro ratio in clean air (from datasheet)
#define MQ7_AUTO_CALIBRATE_ON_START 1  ///< Automatically calibrate Ro on startup

static adc_oneshot_unit_handle_t adc1_handle_internal;
static float mq7_ro = 10.0f;              ///< Initial Ro value (can be adjusted after calibration)

extern void update_mq7_data(float voltage, float ppm);

/**
 * @brief Calculates the sensor resistance (Rs) based on output voltage.
 *
 * Uses the MQ-series formula: Rs = (Vref - Vout) * RL / Vout
 *
 * @param vout The measured voltage output from the sensor.
 * @return Calculated resistance Rs.
 */
static float calculate_rs(float vout) {
    return (MQ7_VREF - vout) * MQ7_RL / vout;
}

/**
 * @brief Converts sensor resistance to CO concentration in ppm.
 *
 * Applies the sensor's response curve: ppm = A * (Rs/Ro)^B
 * with A = 99.042 and B = -1.518 based on datasheet calibration.
 *
 * @param rs The calculated sensor resistance.
 * @return Estimated carbon monoxide concentration in ppm.
 */
static float mq7_rs_to_ppm(float rs) {
    float ratio = rs / mq7_ro;
    return 99.042f * powf(ratio, -1.518f);
}

/**
 * @brief Calibrates the MQ-7 sensor by measuring Ro in clean air.
 *
 * Reads the ADC value, converts it to voltage, calculates Rs, and sets Ro.
 * Logs the calibration results for reference.
 */
void mq7_calibrate(void) {
    int raw;
    if (adc_oneshot_read(adc1_handle_internal, MQ7_ADC_CHANNEL, &raw) != ESP_OK) {
        ESP_LOGE(MQ7_LOG_TAG, "ADC read failed during calibration");
        return;
    }
    float voltage = raw * (MQ7_VREF / MQ7_RESOLUTION);
    float rs = calculate_rs(voltage);
    mq7_ro = rs / MQ7_RO_CLEAN_AIR;
    ESP_LOGI(MQ7_LOG_TAG, "Calibration complete: Ro = %.2f (Raw: %d, V: %.2f, Rs: %.2f)", mq7_ro, raw, voltage, rs);
}

/**
 * @brief Task that continuously reads MQ-7 sensor data.
 *
 * Periodically performs ADC reads, converts values to voltage, calculates Rs,
 * estimates ppm, and logs the results. Updates global/shared data as needed.
 *
 * @param pvParameters Unused task parameter.
 */
static void mq7_read_task(void *pvParameters) {
    while (true) {
        int raw_value = 0;
        if (adc_oneshot_read(adc1_handle_internal, MQ7_ADC_CHANNEL, &raw_value) != ESP_OK) {
            ESP_LOGE(MQ7_LOG_TAG, "ADC read failed");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        float voltage = raw_value * (MQ7_VREF / MQ7_RESOLUTION);
        float rs = calculate_rs(voltage);
        float ppm = mq7_rs_to_ppm(rs);

        ESP_LOGI(MQ7_LOG_TAG, "Raw: %d, Voltage: %.2f V, Rs: %.2f, CO_PPM: %.2f", raw_value, voltage, rs, ppm);
        update_mq7_data(voltage, ppm);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/**
 * @brief Initializes the MQ-7 sensor ADC channel configuration.
 *
 * Sets up ADC channel properties including bit width and attenuation.
 *
 * @param handle A configured `adc_oneshot_unit_handle_t` instance.
 */
void mq7_sensor_init(adc_oneshot_unit_handle_t handle) {
    adc1_handle_internal = handle;

    adc_oneshot_chan_cfg_t channel_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    if (adc_oneshot_config_channel(adc1_handle_internal, MQ7_ADC_CHANNEL, &channel_cfg) == ESP_OK) {
        ESP_LOGI(MQ7_LOG_TAG, "MQ7 ADC channel configured successfully");

    #if MQ7_AUTO_CALIBRATE_ON_START
        mq7_calibrate();
    #endif

    } else {
        ESP_LOGE(MQ7_LOG_TAG, "Failed to configure MQ7 ADC channel");
    }
}

/**
 * @brief Starts the background task that reads from the MQ-7 sensor.
 *
 * Launches a FreeRTOS task responsible for sensor data acquisition and logging.
 */
void mq7_start_read_task(void) {
    xTaskCreate(mq7_read_task, "mq7_read_task", 2048, NULL, 5, NULL);
}

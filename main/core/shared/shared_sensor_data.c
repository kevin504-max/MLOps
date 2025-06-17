/**
 * @file shared_sensor_data.c
 * @brief Thread-safe shared storage for sensor readings.
 *
 * This module provides functions to safely update and retrieve sensor data
 * from multiple sensor reading tasks using a FreeRTOS mutex for synchronization.
 */

#include "shared_sensor_data.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static float last_temperature = 0.0f;
static float last_humidity = 0.0f;
static float last_mq4_voltage = 0.0f;
static float last_mq4_ppm = 0.0f;
static float last_mq7_voltage = 0.0f;
static float last_mq7_ppm = 0.0f;

static SemaphoreHandle_t sensor_data_mutex;

/**
 * @brief Initializes the mutex used to protect sensor data.
 *
 * Must be called before any calls to update or get sensor data.
 */
void shared_sensor_data_init(void) {
    sensor_data_mutex = xSemaphoreCreateMutex();
}

/**
 * @brief Updates the latest DHT sensor data (temperature and humidity).
 *
 * This function locks the mutex before updating shared variables to ensure
 * thread-safe access.
 *
 * @param temperature Latest temperature reading.
 * @param humidity Latest humidity reading.
 */
void update_dht_data(float temperature, float humidity) {
    xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
    last_temperature = temperature;
    last_humidity = humidity;
    xSemaphoreGive(sensor_data_mutex);
}

/**
 * @brief Updates the latest MQ4 sensor data (voltage and ppm).
 *
 * @param voltage Latest MQ4 sensor voltage reading.
 * @param ppm Latest MQ4 sensor ppm reading.
 */
void update_mq4_data(float voltage, float ppm) {
    xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
    last_mq4_voltage = voltage;
    last_mq4_ppm = ppm;
    xSemaphoreGive(sensor_data_mutex);
}

/**
 * @brief Updates the latest MQ7 sensor data (voltage and ppm).
 *
 * @param voltage Latest MQ7 sensor voltage reading.
 * @param ppm Latest MQ7 sensor ppm reading.
 */
void update_mq7_data(float voltage, float ppm) {
    xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
    last_mq7_voltage = voltage;
    last_mq7_ppm = ppm;
    xSemaphoreGive(sensor_data_mutex);
}

/**
 * @brief Retrieves the most recent sensor data for all sensors.
 *
 * Copies the latest sensor values into the provided pointers. Uses mutex
 * to ensure thread-safe access.
 *
 * @param temperature Pointer to store the latest temperature.
 * @param humidity Pointer to store the latest humidity.
 * @param mq4_voltage Pointer to store the latest MQ4 voltage.
 * @param mq4_ppm Pointer to store the latest MQ4 ppm.
 * @param mq7_voltage Pointer to store the latest MQ7 voltage.
 * @param mq7_ppm Pointer to store the latest MQ7 ppm.
 */
void get_sensor_data(
    float *temperature,
    float *humidity,
    float *mq4_voltage,
    float *mq4_ppm,
    float *mq7_voltage,
    float *mq7_ppm
) {
    xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
    *temperature = last_temperature;
    *humidity = last_humidity;
    *mq4_voltage = last_mq4_voltage;
    *mq4_ppm = last_mq4_ppm;
    *mq7_voltage = last_mq7_voltage;
    *mq7_ppm = last_mq7_ppm;
    xSemaphoreGive(sensor_data_mutex);
}
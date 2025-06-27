/**
    * @file shared_sensor_data.h
    * @brief Provides functions to update and retrieve sensor data in a thread-safe manner.
    *
    * This module allows multiple tasks to safely update and access sensor data
    * such as temperature, humidity, and gas sensor readings (MQ4 and MQ7).
    * It uses a mutex to ensure that updates and reads do not conflict.
*/

#ifndef SHARED_SENSOR_DATA_H
#define SHARED_SENSOR_DATA_H

/**
 * @brief Initializes the shared sensor data mutex.
 * 
 * Must be called before using update or get functions.
 */
void shared_sensor_data_init(void);

/**
 * @brief Updates the temperature and humidity values from the DHT sensor.
 * 
 * @param temperature Current temperature reading.
 * @param humidity Current humidity reading.
 */
void update_dht_data(float temperature, float humidity);

/**
 * @brief Updates the voltage and ppm values from the MQ4 sensor.
 * 
 * @param voltage Current MQ4 sensor voltage.
 * @param ppm Current MQ4 sensor ppm reading.
 */
void update_mq4_data(float voltage, float ppm);

/**
 * @brief Updates the voltage and ppm values from the MQ7 sensor.
 * 
 * @param voltage Current MQ7 sensor voltage.
 * @param ppm Current MQ7 sensor ppm reading.
 */
void update_mq7_data(float voltage, float ppm);

/**
 * @brief Retrieves the latest sensor data for all sensors.
 * 
 * Copies the latest sensor values into the provided pointers.
 * 
 * @param temperature Pointer to store temperature.
 * @param humidity Pointer to store humidity.
 * @param mq4_voltage Pointer to store MQ4 sensor voltage.
 * @param mq4_ppm Pointer to store MQ4 sensor ppm.
 * @param mq7_voltage Pointer to store MQ7 sensor voltage.
 * @param mq7_ppm Pointer to store MQ7 sensor ppm.
 */
void get_sensor_data(
    float *temperature,
    float *humidity,
    float *mq4_voltage,
    float *mq4_ppm,
    float *mq7_voltage,
    float *mq7_ppm
);

#endif // SHARED_SENSOR_DATA_H
/**
    * @file dht_sensor.h
    * @brief Header file for DHT sensor initialization and reading task.
    *
    * This module provides functions to initialize the GPIO pin for the DHT sensor
    * and to create a task that periodically reads temperature and humidity data.
*/

#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

/**
 * @brief Initializes the GPIO pin for the DHT sensor.
 */
void dht_sensor_init(void);

/**
 * @brief Creates and starts the task that periodically reads temperature and humidity.
 */
void dht_start_read_task(void);

#endif // DHT_SENSOR_H
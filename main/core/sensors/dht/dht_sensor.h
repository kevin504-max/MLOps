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
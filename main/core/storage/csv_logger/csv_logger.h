/**
 * @file csv_logger.h
 * @brief Interface for logging sensor data to a CSV file in SPIFFS.
 *
 * This module provides functions to create a timestamped CSV file,
 * initialize it with headers, and append rows of sensor data with timestamps.
 */

#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generates a unique CSV filename based on the current system time.
 *
 * The generated filename is stored internally and used for logging.
 * This function should be called before initializing or appending to the CSV.
 */
void create_csv_filename(void);

/**
 * @brief Initializes the CSV file by creating it and writing the header row.
 *
 * If a file with the same name already exists, it will be overwritten.
 */
void init_csv_file(void);

/**
 * @brief Appends a single line of sensor data to the CSV file.
 *
 * @param temperature Temperature in Celsius.
 * @param humidity Humidity in percent.
 * @param mq4_voltage Voltage output from MQ-4 sensor.
 * @param mq4_ppm Estimated PPM reading from MQ-4 sensor.
 * @param mq7_voltage Voltage output from MQ-7 sensor.
 * @param mq7_ppm Estimated CO PPM reading from MQ-7 sensor.
 */
void append_sensor_data_to_csv(float temperature, float humidity, float mq4_voltage, float mq4_ppm, float mq7_voltage, float mq7_ppm);

/**
 * @brief Retrieves the current timestamp in a string format.
 *
 * @param[out] timestamp Buffer to store the resulting timestamp.
 * @param[in] max_len Maximum length of the buffer.
 *
 * Format used: YYYY-MM-DD HH:MM:SS
 */
void get_current_timestamp(char *timestamp, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif // CSV_LOGGER_H
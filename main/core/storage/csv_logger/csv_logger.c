/**
 * @file csv_logger.c
 * @brief Handles CSV logging of sensor data on SPIFFS.
 *
 * This module provides utility functions for creating and managing a CSV file
 * that logs sensor readings with timestamps. The file is saved in the SPIFFS
 * file system and named based on the current date and time.
 */

#include "csv_logger.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define TAG "CSV_LOGGER"

static char csv_file_path[64];  ///< Stores the full path of the CSV file

/**
 * @brief Generates a filename for the CSV log based on the current date and time.
 *
 * The filename format is: /spiffs/data_YY_MM_DD-HH_MM.csv
 * The generated path is stored in the internal static variable csv_file_path.
 */
void create_csv_filename(void) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(csv_file_path, sizeof(csv_file_path), "/spiffs/data_%Y_%m_%d_%H_%M_%S.csv", &timeinfo);
    ESP_LOGI(TAG, "CSV file path: %s", csv_file_path);
}

/**
 * @brief Retrieves the current timestamp in a formatted string.
 *
 * @param[out] timestamp Buffer to hold the resulting timestamp.
 * @param[in] max_len Maximum length of the buffer.
 *
 * The format used is: YYYY-MM-DD HH:MM:SS
 */
void get_current_timestamp(char *timestamp, size_t max_len) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *timeinfo = localtime(&tv.tv_sec);
    strftime(timestamp, max_len, "%Y-%m-%d %H:%M:%S", timeinfo);
}

/**
 * @brief Initializes the CSV log file.
 *
 * Creates a new CSV file (overwriting any existing one with the same name),
 * and writes the header row with column labels.
 */
void init_csv_file(void) {
    ESP_LOGI(TAG, "Initializing CSV file: %s", csv_file_path);

    struct stat st;
    if (stat(csv_file_path, &st) == 0) {
        ESP_LOGW(TAG, "CSV file already exists, overwriting");
    }

    FILE *file = fopen(csv_file_path, "w");
    if (file != NULL) {
        fprintf(file, "Timestamp,Temperature(C),Humidity(%%),MQ4_Voltage(V),MQ4_PPM,MQ7_Voltage(V),MQ7_CO_PPM\n");
        fclose(file);
        ESP_LOGI(TAG, "CSV file initialized successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create CSV file. Error: %s", strerror(errno));
    }
}

/**
 * @brief Appends a row of sensor data to the CSV file.
 *
 * @param temperature Temperature in Celsius.
 * @param humidity Humidity in percent.
 * @param mq4_voltage Voltage from MQ-4 sensor.
 * @param mq4_ppm Estimated gas concentration from MQ-4 (PPM).
 * @param mq7_voltage Voltage from MQ-7 sensor.
 * @param mq7_ppm Estimated CO concentration from MQ-7 (PPM).
 */
void append_sensor_data_to_csv(float temperature, float humidity, float mq4_voltage, float mq4_ppm, float mq7_voltage, float mq7_ppm) {
    FILE *file = fopen(csv_file_path, "a");
    if (file != NULL) {
        char timestamp[64];
        get_current_timestamp(timestamp, sizeof(timestamp));
        fprintf(file, "%s,%.1f,%.1f,%.2f,%.2f,%.2f,%.2f\n", timestamp, temperature, humidity, mq4_voltage, mq4_ppm, mq7_voltage, mq7_ppm);
        fclose(file);
        ESP_LOGD(TAG, "Data appended to CSV");
    } else {
        ESP_LOGE(TAG, "Failed to open CSV file for writing. Error: %s", strerror(errno));
    }
}
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

#include "./core/shared/shared_sensor_data.h"
#include "./core/storage/csv_logger/csv_logger.h"

#define TAG "CSV_WRITER"

/**
 * @brief Task responsible for periodically collecting sensor data and saving it to a CSV file.
 * 
 * This task retrieves the latest temperature, humidity, and gas sensor data (MQ4 and MQ7)
 * from shared memory and appends them to a CSV file every 2 seconds.
 * 
 * @param pvParameters Task parameters (not used).
 */
static void csv_writer_task(void *pvParameters) {
    (void) pvParameters; // Avoid unused parameter warning
    while (true) {
        float temp, hum, mq4_v, mq4_p, mq7_v, mq7_p;
        
        // Get updated sensor data from shared memory
        get_sensor_data(&temp, &hum, &mq4_v, &mq4_p, &mq7_v, &mq7_p);

        // Append sensor data to CSV file
        append_sensor_data_to_csv(temp, hum, mq4_v, mq4_p, mq7_v, mq7_p);

        // Log message to indicate data has been successfully recorded
        ESP_LOGI(TAG, "Sensor data recorded: T=%.2f°C, H=%.2f%%, MQ4=%.2fV/%.2fppm, MQ7=%.2fV/%.2fppm",
                 temp, hum, mq4_v, mq4_p, mq7_v, mq7_p);

        // Delay for 10 seconds before next reading
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief Creates and starts the CSV writer task.
 * 
 * Logs an error if task creation fails.
 */
void start_csv_writer_task(void) {
    if (xTaskCreate(csv_writer_task, "csv_writer_task", 4096, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create CSV writer task");
    }
}
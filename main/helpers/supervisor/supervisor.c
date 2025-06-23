#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"

#include "../../core/storage/csv_merger/csv_merger.h"

#define TAG "SUPERVISOR"

/**
 * @brief Task to monitor runtime and gracefully shutdown.
 *
 * After a defined period, it merges the CSV logs and restarts the device.
 */
void supervisor_task(void *pvParameters) {
    (void) pvParameters;

    const int runtime_minutes = 1;
    const int wait_time_ms = runtime_minutes * 60 * 1000;

    ESP_LOGI(TAG, "Supervisor started. Waiting %d minutes before shutdown...", runtime_minutes);
    vTaskDelay(pdMS_TO_TICKS(wait_time_ms));

    ESP_LOGI(TAG, "Merging CSV files...");
    merge_all_csv_files("/spiffs/merged.csv");

    ESP_LOGW(TAG, "Supervisor task completed. Shutting down the system execution...");
    esp_deep_sleep_start();
}
/**
 * @file spiffs_manager.c
 * @brief Handles initialization and status checking of the SPIFFS filesystem.
 *
 * This module initializes the SPIFFS filesystem, mounts it at the specified path,
 * and logs partition information. It ensures the file system is ready for read/write
 * operations and checks if mounting was successful.
 */

#include "spiffs_manager.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_err.h"
#include <sys/stat.h>

#define TAG "SPIFFS"

/**
 * @brief Initializes and mounts the SPIFFS filesystem.
 *
 * Registers the SPIFFS file system with default configuration, mounts it at "/spiffs",
 * and prints the partition size (total and used). If mounting fails, attempts formatting
 * the partition (if enabled) and logs any errors accordingly.
 */
void init_spiffs(void) {
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        switch (ret) {
            case ESP_FAIL:
                ESP_LOGE(TAG, "Failed to mount or format filesystem");
                break;
            case ESP_ERR_NOT_FOUND:
                ESP_LOGE(TAG, "Failed to find SPIFFS partition");
                break;
            default:
                ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
                break;
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition info (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d bytes, used: %d bytes", total, used);
    }

    struct stat st;
    if (stat("/spiffs", &st) == 0) {
        ESP_LOGI(TAG, "SPIFFS mounted successfully");
    } else {
        ESP_LOGE(TAG, "SPIFFS mount check failed");
    }
}
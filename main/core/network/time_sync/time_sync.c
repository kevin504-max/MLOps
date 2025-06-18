/**
 * @file time_sync.c
 * @brief Handles system time synchronization using SNTP.
 *
 * This module is responsible for initializing the SNTP client and
 * waiting until the system time is correctly set from an NTP server.
 */

#include "time_sync.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>

#define TAG "TIME_SYNC"

/**
 * @brief Initializes the SNTP client with default configuration.
 *
 * Sets the SNTP operating mode to polling and assigns the default NTP server
 * ("pool.ntp.org"). Must be called before attempting to synchronize system time.
 */
void initialize_sntp(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

/**
 * @brief Blocks execution until the system time is synchronized.
 *
 * This function repeatedly checks if the system time has been set
 * (by checking if the year is later than 2020). It waits up to 10 attempts,
 * with a 2-second delay between each attempt. Logs the result after finishing.
 */
bool wait_for_time_sync(void) {
    time_t now;
    struct tm timeinfo;
    int retry = 0;
    const int retry_count = 10;

    setenv("TZ", "UTC+3", 1);
    tzset();

    time(&now);
    localtime_r(&now, &timeinfo);

    while (timeinfo.tm_year < (2020 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    bool wifi_connected = (retry < retry_count);
    if (wifi_connected) {
        ESP_LOGI(TAG, "Time synchronized: %s", asctime(&timeinfo));
    } else {
        ESP_LOGE(TAG, "Failed to synchronize time");
    }

    return wifi_connected;
}
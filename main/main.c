#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <dht.h>
#include <driver/gpio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "esp_spiffs.h"
#include "esp_log.h"
#include "esp_err.h"

#define dht_gpio GPIO_NUM_4
#define dht_type DHT_TYPE_DHT22
#define CSV_FILE "/spiffs/sensor_data.csv"
#define TAG "DHT22_CSV"

void init_spiffs() {
    ESP_LOGI(TAG, "Initializing SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
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

void get_current_timestamp(char *timestamp, size_t max_len) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *timeinfo = localtime(&tv.tv_sec);
    
    strftime(timestamp, max_len, "%Y-%m-%d %H:%M:%S", timeinfo);
    snprintf(timestamp + strlen(timestamp), max_len - strlen(timestamp), ".%03ld", tv.tv_usec / 1000);
}

void init_csv_file() {
    ESP_LOGI(TAG, "Initializing CSV file");
    
    // Primeiro verifica se o arquivo existe
    struct stat st;
    if (stat(CSV_FILE, &st) == 0) {
        ESP_LOGW(TAG, "CSV file already exists, overwriting");
    }

    FILE *file = fopen(CSV_FILE, "w");
    if (file != NULL) {
        fprintf(file, "Timestamp,Temperature(C),Humidity(%%)\n");
        fclose(file);
        ESP_LOGI(TAG, "CSV file initialized successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create CSV file. Error: %s", strerror(errno));
    }
}

void append_to_csv(float temperature, float humidity) {
    FILE *file = fopen(CSV_FILE, "a");
    if (file != NULL) {
        char timestamp[64];
        get_current_timestamp(timestamp, sizeof(timestamp));
        
        fprintf(file, "%s,%.1f,%.1f\n", timestamp, temperature, humidity);
        fclose(file);
        ESP_LOGD(TAG, "Data appended to CSV");
    } else {
        ESP_LOGE(TAG, "Failed to open CSV file for writing. Error: %s", strerror(errno));
    }
}

void dht_test(void *pvParameters) {
    float temperature, humidity;
 
    #ifdef CONFIG_EXAMPLE_INTERNAL_PULLUP
        gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);
    #endif

    while (1) {
        if (dht_read_float_data(dht_type, dht_gpio, &humidity, &temperature) == ESP_OK) {
            ESP_LOGI(TAG, "Humidity: %.1f%%, Temperature: %.1fC", humidity, temperature);
            append_to_csv(temperature, humidity);
        } else {
            ESP_LOGE(TAG, "Could not read data from sensor");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main() {
    init_spiffs();
    init_csv_file();

    xTaskCreate(dht_test, "dht_test", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Application started successfully");
}
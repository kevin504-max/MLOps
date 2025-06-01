#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "esp_adc/adc_oneshot.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_spiffs.h"

#include "./components/dht/dht_sensor.h"
#include "./components/mq4/mq4_sensor.h"
#include "./components/mq7/mq7_sensor.h"

#define CSV_FILE "/spiffs/sensor_data.csv"
#define TAG "MAIN_LOG"

// =========================== SPIFFS ===========================

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

// =========================== CSV ===========================

void get_current_timestamp(char *timestamp, size_t max_len) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *timeinfo = localtime(&tv.tv_sec);

    strftime(timestamp, max_len, "%Y-%m-%d %H:%M:%S", timeinfo);
    snprintf(timestamp + strlen(timestamp), max_len - strlen(timestamp), ".%03ld", tv.tv_usec / 1000);
}

void init_csv_file() {
    ESP_LOGI(TAG, "Initializing CSV file");

    struct stat st;
    if (stat(CSV_FILE, &st) == 0) {
        ESP_LOGW(TAG, "CSV file already exists, overwriting");
    }

    FILE *file = fopen(CSV_FILE, "w");
    if (file != NULL) {
        fprintf(file, "Timestamp,Temperature(C),Humidity(%%),MQ4_Voltage(V),MQ7_Voltage(V),MQ7_CO_PPM\n");
        fclose(file);
        ESP_LOGI(TAG, "CSV file initialized successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create CSV file. Error: %s", strerror(errno));
    }
}

void append_sensor_data_to_csv(float temperature, float humidity, float mq4_voltage, float mq7_voltage, float mq7_ppm) {
    FILE *file = fopen(CSV_FILE, "a");
    if (file != NULL) {
        char timestamp[64];
        get_current_timestamp(timestamp, sizeof(timestamp));

        fprintf(file, "%s,%.1f,%.1f,%.2f,%.2f,%.2f\n", timestamp, temperature, humidity, mq4_voltage, mq7_voltage, mq7_ppm);
        fclose(file);
        ESP_LOGD(TAG, "Data appended to CSV");
    } else {
        ESP_LOGE(TAG, "Failed to open CSV file for writing. Error: %s", strerror(errno));
    }
}

// =========================== DADOS COMPARTILHADOS ===========================

static float last_temperature = 0.0f;
static float last_humidity = 0.0f;
static float last_mq4_voltage = 0.0f;
static float last_mq7_voltage = 0.0f;
static float last_mq7_ppm = 0.0f;

static SemaphoreHandle_t sensor_data_mutex;

void update_dht_data(float temperature, float humidity) {
    xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
    last_temperature = temperature;
    last_humidity = humidity;
    xSemaphoreGive(sensor_data_mutex);
}

void update_mq4_data(float voltage) {
    xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
    last_mq4_voltage = voltage;
    xSemaphoreGive(sensor_data_mutex);
}

void update_mq7_data(float voltage, float ppm) {
    xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
    last_mq7_voltage = voltage;
    last_mq7_ppm = ppm;
    xSemaphoreGive(sensor_data_mutex);
}

static void csv_writer_task(void *pvParameters) {
    while (1) {
        xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);
        float temp = last_temperature;
        float hum = last_humidity;
        float mq4_v = last_mq4_voltage;
        float mq7_v = last_mq7_voltage;
        float mq7_p = last_mq7_ppm;
        xSemaphoreGive(sensor_data_mutex);

        append_sensor_data_to_csv(temp, hum, mq4_v, mq7_v, mq7_p);

        vTaskDelay(pdMS_TO_TICKS(2000)); // escreve a cada 2s
    }
}

// =========================== APP MAIN ===========================

void app_main() {
    sensor_data_mutex = xSemaphoreCreateMutex();

    init_spiffs();
    init_csv_file();

    // Inicializa ADC uma única vez
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    adc_oneshot_unit_handle_t adc1_handle;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    dht_sensor_init();
    mq4_sensor_init(adc1_handle);
    mq7_sensor_init(adc1_handle);
    
    dht_start_read_task();
    mq4_start_read_task();
    mq7_start_read_task();

    xTaskCreate(csv_writer_task, "csv_writer_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Application started successfully");
}

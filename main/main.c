#include <stdio.h>
#include <errno.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "esp_adc/adc_oneshot.h"

#include "esp_log.h"
#include "esp_spiffs.h"

#include "./core/network/time_sync/time_sync.h"
#include "./core/network/webserver/webserver.h"
#include "./core/network/wifi_connector/wifi_connector.h"

#include "./core/storage/csv_logger/csv_logger.h"
#include "./core/storage/csv_writer/csv_writer.h"
#include "./core/storage/spiffs_manager/spiffs_manager.h"

#include "./core/shared/shared_sensor_data.h"

#include "./core/sensors/dht/dht_sensor.h"
#include "./core/sensors/mq4/mq4_sensor.h"
#include "./core/sensors/mq7/mq7_sensor.h"

#include "./helpers/supervisor/supervisor.h"

#define TAG "MAIN_LOG"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_sntp.h"

void app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS Flash initialized successfully");

    wifi_connect();
    initialize_sntp();
    if (!wait_for_time_sync()) {
        ESP_LOGE(TAG, "System time not synchronized, aborting startup.");
        return;
    }

    shared_sensor_data_init();
    init_spiffs();

    create_csv_filename();
    init_csv_file();
    start_csv_writer_task();

    start_webserver();

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


    xTaskCreate(supervisor_task, "supervisor_task", 4096, NULL, 1, NULL);
    ESP_LOGI(TAG, "Application started successfully");
}
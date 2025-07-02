/**
 * @file dht_sensor.c
 * @brief Driver for reading temperature and humidity from a DHT sensor (e.g., DHT22).
 *
 * This module initializes the GPIO pin for the DHT sensor and creates a FreeRTOS task
 * to periodically read temperature and humidity data. It sends the readings to
 * the main system via an external callback function.
 */

#include "dht_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h"

#define DHT_LOG_TAG "DHT_SENSOR"

// External function to update sensor data in the main system
extern void update_dht_data(float temperature, float humidity);

// GPIO pin connected to the DHT sensor data line
#define DHT_GPIO_PIN GPIO_NUM_4

// Type of the DHT sensor (DHT11, DHT22, etc.)
#define DHT_SENSOR_TYPE DHT_TYPE_DHT22

/**
 * @brief Task that reads data from the DHT sensor every 10 seconds.
 * 
 * Reads temperature and humidity, logs the results, and calls
 * the external update function.
 * 
 * @param pvParameters Unused task parameter.
 */
static void dht_read_task(void *pvParameters) {
    float temperature = 0.0f;
    float humidity = 0.0f;

    gpio_set_pull_mode(DHT_GPIO_PIN, GPIO_PULLUP_ONLY);

    while (true) {
        esp_err_t result = dht_read_float_data(DHT_SENSOR_TYPE, DHT_GPIO_PIN, &humidity, &temperature);

        if (result == ESP_OK) {
            ESP_LOGI(DHT_LOG_TAG, "Temperature: %.1f°C, Humidity: %.1f%%", temperature, humidity);
            update_dht_data(temperature, humidity);
        } else {
            ESP_LOGE(DHT_LOG_TAG, "Failed to read data from DHT sensor");
        }

        // vTaskDelay(pdMS_TO_TICKS(10000));
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/**
 * @brief Initializes the GPIO pin for the DHT sensor.
 * 
 * Configures the pin as input/output with pull-up enabled.
 */
void dht_sensor_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DHT_GPIO_PIN),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
    gpio_set_pull_mode(DHT_GPIO_PIN, GPIO_PULLUP_ONLY);
}

/**
 * @brief Starts the FreeRTOS task that reads the DHT sensor.
 */
void dht_start_read_task(void) {
    xTaskCreate(dht_read_task, "dht_read_task", 4096, NULL, 5, NULL);
}
#include "dht_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h"

#define DHT_LOG_TAG "DHT_SENSOR"

// Função externa para enviar dados ao sistema principal
extern void update_dht_data(float temperature, float humidity);

// Pino e tipo de sensor configuráveis por macros (pode vir do menuconfig futuramente)
#define DHT_GPIO_PIN GPIO_NUM_4
#define DHT_SENSOR_TYPE DHT_TYPE_DHT22

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

        vTaskDelay(pdMS_TO_TICKS(2000));  // Aguarda 2 segundos entre leituras
    }
}

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

void dht_start_read_task(void) {
    xTaskCreate(dht_read_task, "dht_read_task", 4096, NULL, 5, NULL);
}
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <dht.h>
#include <driver/gpio.h>

#define dht_gpio GPIO_NUM_4
#define dht_type DHT_TYPE_DHT22

void dht_test(void *pvParameters)
{
    float temperature, humidity;
 
    #ifdef CONFIG_EXAMPLE_INTERNAL_PULLUP
        gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);
    #endif

    while (1) {
        if (dht_read_float_data(dht_type, dht_gpio, &humidity, &temperature) == ESP_OK) {
            printf("Humidity: %.1f%% Temperature: %.1fC\n", humidity, temperature);
        } else {
            printf("Could not read data from sensor\n");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main()
{
    xTaskCreate(dht_test, "dht_test", 2048, NULL, 5, NULL);
}
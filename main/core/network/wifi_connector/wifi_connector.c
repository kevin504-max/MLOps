/**
    * @file wifi_connector.c
    * @brief Manages Wi-Fi connection in station mode using ESP-IDF.
    *
    * This module initializes the Wi-Fi driver, connects to a specified Access Point,
    * and handles Wi-Fi and IP events to maintain the connection and log status.
*/

#include "wifi_connector.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "sdkconfig.h"

#define WIFI_SSID "CATIVEIRO"
#define WIFI_PASS "Catshow1000grau"

static const char *TAG = "WIFI";

/**
 * @brief Wi-Fi and IP event handler
 *
 * Callback function triggered by the system when Wi-Fi or IP events occur.
 * Manages Wi-Fi connection states, automatic reconnection, and logs information.
 *
 * @param arg Optional user data (not used)
 * @param event_base Base event type (e.g., WIFI_EVENT or IP_EVENT)
 * @param event_id Specific event ID
 * @param event_data Additional event data (e.g., obtained IP address)
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch(event_id) {
            case WIFI_EVENT_STA_START:
                // Start connecting when Wi-Fi starts
                esp_wifi_connect();
                ESP_LOGI(TAG, "Wi-Fi started, connecting...");
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                // If disconnected, retry connection automatically
                ESP_LOGW(TAG, "Disconnected, retrying...");
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_CONNECTED:
                // Successfully connected to the Access Point
                ESP_LOGI(TAG, "Connected to AP");
                break;

            default:
                // Other Wi-Fi events can be handled here if needed
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // Log the IP address obtained via DHCP
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

/**
 * @brief Initialize and connect to Wi-Fi in station mode
 *
 * Sets up the TCP/IP stack, registers the event handler to manage
 * the Wi-Fi connection lifecycle, and starts the connection with the defined credentials.
 */
void wifi_connect(void) {
    // Initialize the TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Create the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create default Wi-Fi station interface
    esp_netif_create_default_wifi_sta();

    // Initialize Wi-Fi driver with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handler for Wi-Fi and IP events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Set Wi-Fi credentials
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    // Set Wi-Fi mode to station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Apply Wi-Fi configuration (SSID and password)
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Start the Wi-Fi driver
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialization done.");
}
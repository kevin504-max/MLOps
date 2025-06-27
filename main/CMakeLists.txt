idf_component_register(
    SRCS 
        "./core/network/time_sync/time_sync.c"
        "./core/network/webserver/webserver.c"
        "./core/network/wifi_connector/wifi_connector.c"

        "./core/sensors/dht/dht_sensor.c"
        "./core/sensors/mq7/mq7_sensor.c"
        "./core/sensors/mq4/mq4_sensor.c"
        
        "./core/storage/csv_logger/csv_logger.c"
        "./core/storage/csv_merger/csv_merger.c"
        "./core/storage/csv_writer/csv_writer.c"
        "./core/storage/spiffs_manager/spiffs_manager.c"

        "./core/shared/shared_sensor_data.c"

        "./helpers/supervisor/supervisor.c"

        "main.c"
    
    INCLUDE_DIRS 
        "."
        "./core/network"
        "./core/sensors"
        "./core/shared"
        "./core/storage"
        "./helpers"
    
    REQUIRES 
        dht 
        esp_adc
        lwip 
        
    PRIV_REQUIRES
        esp_http_server
        spiffs
        esp_wifi
        esp_netif
        nvs_flash
        esp_event
)
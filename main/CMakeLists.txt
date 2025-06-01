idf_component_register(SRCS "./components/dht/dht_sensor.c" "./components/mq7/mq7_sensor.c" "./components/mq4/mq4_sensor.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES dht esp_adc
                    PRIV_REQUIRES spiffs)
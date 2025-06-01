#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa o pino GPIO usado pelo sensor DHT.
 */
void dht_sensor_init(void);

/**
 * @brief Cria a task de leitura periódica do sensor DHT.
 */
void dht_start_read_task(void);

#ifdef __cplusplus
}
#endif

#endif
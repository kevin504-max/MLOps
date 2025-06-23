#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_err.h"
#include "esp_http_server.h"

/**
 * @brief Inicializa e inicia o servidor HTTP com endpoint para download do arquivo merged.csv.
 * 
 * @return httpd_handle_t Handle do servidor HTTP iniciado, ou NULL em caso de erro.
 */
httpd_handle_t start_webserver(void);

#endif // HTTP_SERVER_H

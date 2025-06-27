/**
    * @file webserver.h
    * @brief Provides an HTTP endpoint to download CSV data from the filesystem.
    *
    * This module sets up a lightweight HTTP server using ESP-IDF's `esp_http_server`
    * to serve a CSV file located in the filesystem. The server listens for GET requests
    * on the root path and responds with the contents of `merged.csv`.
*/

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

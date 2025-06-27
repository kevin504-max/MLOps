/**
 * @file webserver.c
 * @brief Provides an HTTP endpoint to download CSV data from the filesystem.
 *
 * This module sets up a lightweight HTTP server using ESP-IDF's `esp_http_server`
 * and provides a `/download_csv` endpoint. When accessed, it streams the contents
 * of a CSV file stored in the SPIFFS filesystem to the client as a downloadable file.
 */

#include "esp_http_server.h"
#include "esp_log.h"
#include <stdio.h>

#define TAG "HTTP_SERVER"
#define MERGED_CSV_PATH "/spiffs/merged.csv"  ///< Path to the CSV file to be served

/**
 * @brief HTTP GET handler for downloading the merged CSV file.
 *
 * This function opens the specified CSV file and streams it in chunks to the client.
 * It sets the appropriate headers so that the browser understands it as a downloadable file.
 *
 * @param req Pointer to the HTTP request object.
 * @return ESP_OK on success, ESP_FAIL on error.
 */
static esp_err_t download_csv_handler(httpd_req_t *req) {
    FILE *file = fopen(MERGED_CSV_PATH, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file: %s", MERGED_CSV_PATH);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Set MIME type and attachment header
    httpd_resp_set_type(req, "text/csv");
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment; filename=\"merged.csv\"");

    // Stream file content in 512-byte chunks
    char buffer[512];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (httpd_resp_send_chunk(req, buffer, read_bytes) != ESP_OK) {
            fclose(file);
            ESP_LOGE(TAG, "Error sending file chunk");
            httpd_resp_sendstr_chunk(req, NULL);  // Terminate the response
            return ESP_FAIL;
        }
    }

    // Finalize the response
    httpd_resp_send_chunk(req, NULL, 0);
    fclose(file);
    return ESP_OK;
}

/**
 * @brief Starts the embedded HTTP server and registers the download URI handler.
 *
 * Configures and launches the HTTP server, making the `/download_csv` endpoint available.
 *
 * @return The server handle on success, NULL on failure.
 */
httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting HTTP Server");
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t download_uri = {
            .uri       = "/download_csv",
            .method    = HTTP_GET,
            .handler   = download_csv_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &download_uri);
        return server;
    }

    ESP_LOGE(TAG, "Failed to start HTTP Server");
    return NULL;
}

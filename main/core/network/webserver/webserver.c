#include "esp_http_server.h"
#include "esp_log.h"
#include <stdio.h>

#define TAG "HTTP_SERVER"
#define MERGED_CSV_PATH "/spiffs/merged.csv"

static esp_err_t download_csv_handler(httpd_req_t *req) {
    FILE *file = fopen(MERGED_CSV_PATH, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file: %s", MERGED_CSV_PATH);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Define o header para o navegador entender que é um arquivo para download
    httpd_resp_set_type(req, "text/csv");
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment; filename=\"merged.csv\"");

    char buffer[512];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (httpd_resp_send_chunk(req, buffer, read_bytes) != ESP_OK) {
            fclose(file);
            ESP_LOGE(TAG, "Error sending file chunk");
            httpd_resp_sendstr_chunk(req, NULL); // indica fim
            return ESP_FAIL;
        }
    }
    httpd_resp_send_chunk(req, NULL, 0);  // Indica fim da resposta

    fclose(file);
    return ESP_OK;
}

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
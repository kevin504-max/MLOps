/**
    * @file csv_merger.c
    * @brief Merges multiple CSV files in the SPIFFS directory into a single output file.
    *
    * This module provides functionality to read all CSV files in the SPIFFS directory,
    * skip their headers (except for the first file), and write the combined data into a single output file.
*/

#include "csv_merger.h"
#include "esp_log.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define TAG "CSV_MERGER"
#define CSV_DIR "/spiffs"

/**
 * @brief Merges all .csv files in the SPIFFS directory into a single output file.
 * 
 * The function opens each .csv file in the /spiffs directory, skips the header of each (except the first),
 * and appends the remaining lines into the output file.
 */
void merge_all_csv_files(const char *output_filename) {
    DIR *dir = opendir(CSV_DIR);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory: %s", CSV_DIR);
        return;
    }

    FILE *output = fopen(output_filename, "w");
    if (!output) {
        ESP_LOGE(TAG, "Failed to create output file: %s", output_filename);
        closedir(dir);
        return;
    }

    struct dirent *entry;
    int file_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Skip non-csv files
        if (!strstr(entry->d_name, ".csv")) continue;
        // Skip the output file if it already exists in /spiffs
        if (strcmp(entry->d_name, output_filename + strlen(CSV_DIR) + 1) == 0) continue;

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", CSV_DIR, entry->d_name);
        FILE *input = fopen(file_path, "r");
        if (!input) {
            ESP_LOGW(TAG, "Failed to open file: %s", file_path);
            continue;
        }

        ESP_LOGI(TAG, "Merging file: %s", file_path);

        char line[256];
        bool skip_header = file_count > 0;

        while (fgets(line, sizeof(line), input)) {
            // Skip header from second file onward
            if (skip_header && strstr(line, "Timestamp")) continue;
            fputs(line, output);
        }

        fclose(input);
        file_count++;
    }

    fclose(output);
    closedir(dir);
    ESP_LOGI(TAG, "Merged %d files into %s", file_count, output_filename);
}
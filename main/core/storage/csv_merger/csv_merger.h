#ifndef CSV_MERGER_H
#define CSV_MERGER_H

/**
 * @brief Merges all CSV files in /spiffs into a single CSV file.
 * 
 * @param output_filename The full path to the output CSV file (e.g. "/spiffs/merged.csv").
 */
void merge_all_csv_files(const char *output_filename);

#endif // CSV_MERGER_H
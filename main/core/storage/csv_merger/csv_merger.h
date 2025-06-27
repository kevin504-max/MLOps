/**
    * @file csv_merger.h
    * @brief Header file for merging multiple CSV files in SPIFFS into a single output file.
    *
    * This module provides a function to read all CSV files in the SPIFFS directory,
    * skip their headers (except for the first file), and write the combined data into a single output file.
*/

#ifndef CSV_MERGER_H
#define CSV_MERGER_H

/**
 * @brief Merges all CSV files in /spiffs into a single CSV file.
 * 
 * @param output_filename The full path to the output CSV file (e.g. "/spiffs/merged.csv").
 */
void merge_all_csv_files(const char *output_filename);

#endif // CSV_MERGER_H
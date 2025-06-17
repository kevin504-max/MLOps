#ifndef CSV_WRITER_H
#define CSV_WRITER_H

/**
 * @brief Starts the FreeRTOS task that periodically collects sensor data and writes it to a CSV file.
 * 
 * This function creates the csv_writer_task, which runs indefinitely,
 * logging sensor data every 2 seconds.
 */
void start_csv_writer_task(void);

#endif // CSV_WRITER_H
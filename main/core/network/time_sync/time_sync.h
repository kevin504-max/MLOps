/**
 * @file time_sync.h
 * @brief Header file for SNTP time synchronization functions.
 *
 * This module provides functions to initialize the SNTP client
 * and wait for the system time to be synchronized using NTP.
 */

#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <stdbool.h>

/**
 * @brief Initializes the SNTP client.
 *
 * Configures SNTP to use polling mode and sets the default NTP server.
 */
void initialize_sntp(void);

/**
 * @brief Waits until the system time is synchronized via SNTP.
 *
 * Polls the system time and blocks until it is correctly set
 * or the retry limit is reached.
 */
bool wait_for_time_sync(void);

#endif // TIME_SYNC_H
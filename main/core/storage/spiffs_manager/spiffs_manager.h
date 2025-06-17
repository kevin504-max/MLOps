#ifndef SPIFFS_MANAGER_H
#define SPIFFS_MANAGER_H

/**
 * @file spiffs_manager.h
 * @brief SPIFFS filesystem management interface.
 *
 * Provides initialization and mounting of the SPIFFS filesystem.
 */

/**
 * @brief Initializes and mounts the SPIFFS filesystem.
 *
 * This function configures the SPIFFS partition, mounts it on "/spiffs",
 * and logs relevant status information about the filesystem.
 * If mounting fails, it may format the partition if configured to do so.
 */
void init_spiffs(void);

#endif // SPIFFS_MANAGER_H
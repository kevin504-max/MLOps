#ifndef WIFI_CONNECTOR_H
#define WIFI_CONNECTOR_H

/**
 * @file wifi_connector.h
 * @brief Provides a function to initialize and connect to Wi-Fi in station mode.
 *
 * This module handles the initialization of the Wi-Fi stack and
 * the connection to a predefined access point using STA mode.
 * It also sets up event handling to manage reconnection and IP acquisition.
 */

/**
 * @brief Initializes the Wi-Fi in station mode and connects to the configured access point.
 *
 * This function sets up the necessary network interface, event loop,
 * and registers event handlers to manage connection and disconnection events.
 * It uses the predefined SSID and password configured in the implementation file.
 */
void wifi_connect(void);

#endif // WIFI_CONNECTOR_H
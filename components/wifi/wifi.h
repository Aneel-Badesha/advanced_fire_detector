#ifndef WIFI_H
#define WIFI_H

#include <esp_err.h>
#include <stdbool.h>

// Initialize WiFi in station mode
esp_err_t wifi_init_sta(const char *ssid, const char *password);

// Check if WiFi is connected
bool wifi_is_connected(void);

// Get IP address as string
const char* wifi_get_ip_address(void);

// Send HTTP POST message to Raspberry Pi
esp_err_t wifi_send_message(const char *message);

// Set Raspberry Pi IP address and port
void wifi_set_rpi_address(const char *ip, int port);

#endif // WIFI_H

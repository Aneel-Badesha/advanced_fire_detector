# Fire Detector

Advanced Fire Detector made in C and Python using ESP-32 and Raspberry-Pi 5

## Description

ESP-IDF project for fire detection system that monitors flame sensors using a 5-second rolling average algorithm and sends HTTP POST alerts to a Raspberry Pi server over WiFi. The system uses FreeRTOS tasks with mutex synchronization for reliable multi-threaded operation.

## Hardware Requirements

- ESP32 development board
- Flame sensor module (GPIO34 - ADC1_CHANNEL_6)
- Push button for manual alarm reset (GPIO23)
- LED status indicator (GPIO2)
- Raspberry Pi 5 running Flask HTTP server on port 5000

## Features

- **5-second rolling average algorithm**: Reduces false alarms by averaging last 5 sensor readings
- **1-second polling interval**: Flame sensor sampled every second via ADC (12-bit resolution)
- **Configurable threshold**: Alarm triggers when average flame intensity ≥ 20
- **WiFi connectivity**: WPA2-PSK authentication with automatic connection management
- **HTTP POST alerts**: JSON payload sent to Raspberry Pi Flask server (192.168.1.90:5000)
- **LED status indicator**: Flashes at 500ms intervals when alarm triggered
- **Button-based reset**: Manual alarm reset via GPIO23 button
- **FreeRTOS architecture**: 4 concurrent tasks (sensor_read, send_message, led_flash, button_reset) with mutex synchronization
- **Circular buffer**: Efficient memory usage for flame intensity history

## Build and Flash

```bash
cd C:\fire_detector
idf.py build
idf.py flash monitor
```

## Configuration

### WiFi Settings
Update in `main/fire_detector.c` (lines 94-95):
```c
const char* ssid = "Wifi Name";
const char* password = "Wifi Password";
```

### Raspberry Pi Server
Configured in `send_message_task()` (line 103):
```c
wifi_set_rpi_address("192.168.1.90", 5000);
```

### Sensor Thresholds
Adjust in `main/fire_detector.c`:
```c
#define FLAME_BUFFER_SIZE 5    // Number of samples in rolling average
#define FLAME_THRESHOLD 20      // Alarm trigger threshold
```

### Polling Interval
Modify sensor read delay (line 81):
```c
vTaskDelay(pdMS_TO_TICKS(1000));  // 1000ms = 1 second polling
```

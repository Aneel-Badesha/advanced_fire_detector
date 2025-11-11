# Advanced Fire Detector

Advanced Fire Detector made in C and Python using ESP-32 and Raspberry-Pi 5

## Description

ESP-IDF project for fire detection system that monitors flame sensors and sends alerts to a Raspberry Pi over WiFi.

## Hardware Requirements

- ESP32 development board
- LM393 flame sensor module (connected to GPIO34)
- Button for manual reset (GPIO23)
- LED indicator (GPIO2)
- Raspberry Pi 5 with Flask server

## Features

- Real-time flame detection using ADC
- WiFi connectivity with automatic reconnection
- HTTP POST alerts to Raspberry Pi
- LED status indicator
- Button-based alarm reset
- FreeRTOS multi-task architecture with mutex synchronization

## Build and Flash

```bash
cd C:\Espressif\frameworks\esp-idf-v5.5.1\projects\fire_detector
idf.py build
idf.py flash monitor
```

## Configuration

Update WiFi credentials in `main/fire_detector.c`:
- SSID: "******"
- Password: "******"
- Raspberry Pi IP: 192.168.1.90:5000

#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sensor.h"
#include "wifi.h"
#include "freertos/semphr.h"

static const char *TAG = "fire_detector";

typedef enum {
    NO_ALARM = 0,
    ALARM_TRIGGERED = 1
} trigger_states;

trigger_states trigger = NO_ALARM;

// Buffer for averaging flame readings
// 5 samples at 1 second intervals
#define FLAME_BUFFER_SIZE 5
#define FLAME_THRESHOLD 20

uint32_t flame_buffer[FLAME_BUFFER_SIZE] = {0};
uint32_t buffer_index = 0;
bool buffer_full = false;

SemaphoreHandle_t mutex;

void sensor_read_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting sensor read task");

    // Initialize flame sensor (ADC)
    adc_config_t adc_flame = {
        .unit = ADC_UNIT_1,
        .channel = FLAME_SENSOR,
        .width = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12
    };

    adc_init(&adc_flame);
    
    while (1) {
        // Read flame sensor
        uint32_t flame_raw = adc_read_raw(&adc_flame);
        uint32_t flame_voltage = adc_raw_to_voltage(flame_raw);
        uint32_t flame_intensity = voltage_to_flame_intensity(flame_voltage);

        // Add current reading to circular buffer
        flame_buffer[buffer_index] = flame_intensity;
        buffer_index = (buffer_index + 1) % FLAME_BUFFER_SIZE;
        
        // Mark buffer as full after first complete cycle
        if (buffer_index == 0 && !buffer_full) {
            buffer_full = true;
        }

        // Calculate average of readings in buffer
        uint32_t sum = 0;
        uint32_t count = buffer_full ? FLAME_BUFFER_SIZE : buffer_index;
        
        for (int i = 0; i < count; i++) {
            sum += flame_buffer[i];
        }
        
        uint32_t avg_flame_intensity = (count > 0) ? (sum / count) : 0;
        
        // Check if average exceeds threshold
        if (avg_flame_intensity >= FLAME_THRESHOLD)
        {
            ESP_LOGI("flame sensor", "Flame Detected, Alarm triggered");
            
            if (xSemaphoreTake(mutex, portMAX_DELAY)) {
                // Critical section: safe to access shared resource
                trigger = ALARM_TRIGGERED;
                xSemaphoreGive(mutex); // release mutex
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void send_message_task(void *pvParameter) {
    ESP_LOGI(TAG, "Sending message to raspi");
    esp_err_t err = ESP_OK;

    if (!wifi_is_connected()) {
        ESP_LOGE(TAG, "ESP32 not connected to wifi");
    }

    const char* ssid = "BadeshaHome";
    const char* password = "Canucks@2011";
    const char* message = "Alarm Triggered";

    err = wifi_init_sta(ssid, password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Wifi init failed: %s", esp_err_to_name(err));
    }

    // Set Raspberry Pi address
    wifi_set_rpi_address("192.168.1.90", 5000);

    while (1) {
        // Check if alarm triggered
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            // Critical section: safe to access shared resource
            if (trigger == ALARM_TRIGGERED) {
                err = wifi_send_message(message);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to send message: %s", esp_err_to_name(err));
                }
                trigger = NO_ALARM;  // Reset trigger after sending message
            }
            xSemaphoreGive(mutex); // release mutex
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void led_flash_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting LED flash task on GPIO %d", LED_PIN);

    gpio_init(LED_PIN, true);
    uint32_t level = 1;

    while (1) {
        
        // Check if alarm triggered
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            // Critical section: safe to access shared resource
            if (trigger == ALARM_TRIGGERED) {
                gpio_write(LED_PIN, level);
                level = !level;   
            }
            xSemaphoreGive(mutex); // release mutex
        }
         
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void button_reset_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting button reset task on GPIO %d", BUTTON_PIN);

    gpio_init(BUTTON_PIN, false);  // Initialize as INPUT to read button state
    
    while (1) {

        // Check if button pressed
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            // Critical section: safe to access shared resource
            if (gpio_read(BUTTON_PIN) == 0) {
                trigger = NO_ALARM;
                gpio_write(LED_PIN, 0);
                ESP_LOGI(TAG, "System reset alarm turned off");
            }
            xSemaphoreGive(mutex); // release mutex
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Fire Detector System Starting...");
    ESP_LOGI(TAG, "System initialized successfully");

    mutex = xSemaphoreCreateMutex(); // create mutex
    if (mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
    }

    xTaskCreate(sensor_read_task, "sensor_read", 4096, NULL, 5, NULL);
    xTaskCreate(send_message_task, "wifi", 4096, NULL, 5, NULL);
    xTaskCreate(led_flash_task, "led", 4096, NULL, 5, NULL );
    xTaskCreate(button_reset_task, "reset", 4096, NULL, 5, NULL );

    vTaskDelete(NULL);
}

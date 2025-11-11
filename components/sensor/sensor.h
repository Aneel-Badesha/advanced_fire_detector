#ifndef SENSOR_H
#define SENSOR_H

#include <esp_adc/adc_oneshot.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"

#define FLAME_SENSOR ADC_CHANNEL_6  // GPIO34
#define LED_PIN      GPIO_NUM_2     // onboard LED
#define BUTTON_PIN   GPIO_NUM_23    // button

// ADC configuration
#define ADC_MAX_VALUE 4095.0f  // 12-bit ADC
#define ADC_VREF 3.3f          // Reference voltage

typedef struct {
    adc_unit_t unit;
    adc_channel_t channel;
    adc_bitwidth_t width;
    adc_atten_t atten;
} adc_config_t;

// Initialize ADC channel
esp_err_t adc_init(const adc_config_t *cfg);

// Read raw ADC value (0 - max based on bit width)
uint32_t adc_read_raw(const adc_config_t *cfg);

// Convert raw ADC value to voltage
uint32_t adc_raw_to_voltage(uint32_t raw);

// Convert LM393 flame sensor voltage to flame intensity percentage
// LM393 outputs LOW (near 0V) when flame detected, HIGH (near 3.3V) when no flame
uint32_t voltage_to_flame_intensity(uint32_t voltage);

// Initialize GPIO
esp_err_t gpio_init(gpio_num_t gpio_num, bool output);

// Write to GPIO port
esp_err_t gpio_write(gpio_num_t gpio_num, uint32_t level);

// Reset GPIO pin
esp_err_t gpio_deinit(gpio_num_t gpio_num);

// Read in GPIO port
uint32_t gpio_read(gpio_num_t gpio_num);

#endif // SENSOR_H

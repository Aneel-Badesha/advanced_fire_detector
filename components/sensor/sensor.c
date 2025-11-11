#include "esp_log.h"
#include "sensor.h"

static const char *TAG = "SENSOR";

// Global ADC oneshot handle
static adc_oneshot_unit_handle_t adc1_handle = NULL;

esp_err_t adc_init(const adc_config_t *cfg) {
    if (!cfg) return ESP_ERR_INVALID_ARG;

    esp_err_t err = ESP_OK;

    // Initialize ADC unit if not already done
    if (cfg->unit == ADC_UNIT_1 && adc1_handle == NULL) {
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
        };
        err = adc_oneshot_new_unit(&init_config, &adc1_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADC unit init failed: %s", esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(TAG, "ADC1 unit initialized");
    }

    // Configure the specific channel
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = cfg->width,
        .atten = cfg->atten,
    };

    if (cfg->unit == ADC_UNIT_1) {
        err = adc_oneshot_config_channel(adc1_handle, cfg->channel, &config);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADC channel config failed: %s", esp_err_to_name(err));
            return err;
        }
    }
    return ESP_OK;
}

uint32_t adc_read_raw(const adc_config_t *cfg) {
    uint32_t raw = 0;
    if (cfg->unit == ADC_UNIT_1) {
        adc_oneshot_read(adc1_handle, cfg->channel, (int *)&raw);
    }
    return raw;
}

uint32_t adc_raw_to_voltage(uint32_t raw) {
    return (uint32_t)(raw / ADC_MAX_VALUE) * ADC_VREF;
}

uint32_t voltage_to_flame_intensity(uint32_t voltage) {
    // Invert: higher voltage = no flame , lower voltage = flame detected 
    uint32_t intensity = (uint32_t)((ADC_VREF - voltage) / ADC_VREF) * 100;
        
    return intensity;
}

esp_err_t gpio_init(gpio_num_t gpio_num, bool output)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT,
        .pull_up_en = output ? GPIO_PULLUP_DISABLE : GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_LOGD(TAG, "GPIO initialized as %s on pin %d", output ? "OUTPUT" : "INPUT", gpio_num);
    return gpio_config(&cfg);
}

esp_err_t gpio_write(gpio_num_t gpio_num, uint32_t level)
{
    return gpio_set_level(gpio_num, level);
}

esp_err_t gpio_deinit(gpio_num_t gpio_num)
{
    return gpio_reset_pin(gpio_num);
}

uint32_t gpio_read(gpio_num_t gpio_num)
{
    return gpio_get_level(gpio_num);
}
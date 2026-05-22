/*
 * Reproducción de audio vía PWM para ESP32-C6
 * Solución: Tarea de alta prioridad (sin usar interrupciones)
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "audio_data.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (2)
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY          (100000)

static const char *TAG = "AUDIO_TASK";

void setup_pwm() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 128,
        .hpoint         = 0,
    };
    ledc_channel_config(&ledc_channel);
}

// Tarea dedicada para el audio
void audio_playback_task(void *pvParameters) {
    uint32_t delay_us = 1000000 / audio_sample_rate;
    ESP_LOGI(TAG, "Tarea de audio iniciada");

    while (1) {
        for (uint32_t i = 0; i < audio_samples_len; i++) {
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, audio_samples[i]);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

            // Respiración del sistema: cada 1000 muestras cedemos 1 tick
            if (i % 1000 == 0) {
                vTaskDelay(1);
            }

            esp_rom_delay_us(delay_us);
        }
        
        // Pausa entre repeticiones del audio
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    setup_pwm();

    // Creamos la tarea con prioridad alta (10)
    xTaskCreate(audio_playback_task, "audio_task", 4096, NULL, 10, NULL);
    
    ESP_LOGI(TAG, "app_main finalizado, la tarea de audio toma el control");
}
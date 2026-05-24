/*
Reproducción de audio vía PWM para ESP32-C6
Audio mono 8-bit a 22050 Hz
*/

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "audio_data.h"

#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO  2
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_DUTY_RES   LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY  37000

#define AUDIO_SAMPLE_RATE 11025


static const char *TAG = "AUDIO_TASK";

void setup_pwm(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .gpio_num = LEDC_OUTPUT_IO,
        .duty = 128,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void audio_playback_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Tarea de audio iniciada");
    ESP_LOGI(TAG, "Sample rate: %d Hz", AUDIO_SAMPLE_RATE);
    ESP_LOGI(TAG, "Muestras: %lu", (unsigned long)audio_samples_len);

    const int64_t total_duration_us = ((int64_t)audio_samples_len * 1000000LL) / AUDIO_SAMPLE_RATE;

    while (1) {
        int64_t start_time = esp_timer_get_time();
        uint32_t last_index = 0xFFFFFFFF;

        while (1) {
            int64_t now = esp_timer_get_time();
            int64_t elapsed = now - start_time;

            if (elapsed >= total_duration_us) {
                break;
            }

            uint32_t index = (uint32_t)((elapsed * AUDIO_SAMPLE_RATE) / 1000000LL);

            if (index >= audio_samples_len) {
                index = audio_samples_len - 1;
            }

            if (index != last_index) {
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, audio_samples[index]);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
                last_index = index;
            }
        }

        int64_t end_time = esp_timer_get_time();

        ESP_LOGI(TAG, "Duracion real: %.3f s", (end_time - start_time) / 1000000.0);

        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 128);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void app_main(void)
{
    setup_pwm();

    xTaskCreatePinnedToCore(
        audio_playback_task,
        "audio_task",
        4096,
        NULL,
        configMAX_PRIORITIES - 1,
        NULL,
        0
    );

    ESP_LOGI(TAG, "app_main finalizado");
}

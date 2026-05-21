/*
 * Copyright (c) 2020 Seagate Technology LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Modified for audio playback via PWM to LM386N amplifier
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

#include "audio_data.h"

#define PWM_CHANNEL 0
#define PWM_FREQ_HZ 100000
#define PWM_PERIOD_US (1000000 / PWM_FREQ_HZ)

/**
 * @brief Reproduce audio desde las muestras incrustadas
 */
static void play_audio(const struct device *pwm_dev)
{
	int err;
	uint32_t pulse_us;

	LOG_INF("Starting audio playback: %d samples at %d Hz", 
		audio_samples_len, audio_sample_rate);
	
	for (uint32_t i = 0; i < audio_samples_len; i++) {
		pulse_us = (audio_samples[i] * PWM_PERIOD_US) / 256;

		err = pwm_set(pwm_dev, PWM_CHANNEL, PWM_PERIOD_US, pulse_us, PWM_POLARITY_NORMAL);
		if (err < 0) {
			LOG_ERR("PWM set failed: %d", err);
			break;
		}

		k_sleep(K_USEC(1000000 / audio_sample_rate));
	}

	pwm_set(pwm_dev, PWM_CHANNEL, PWM_PERIOD_US, PWM_PERIOD_US / 2, PWM_POLARITY_NORMAL);
	LOG_INF("Audio playback complete");
}

int main(void)
{
	const struct device *pwm_dev;
	int err;

	LOG_INF("Audio playback on LM386N via PWM");
	LOG_INF("Pin: P1.11, Frequency: %d Hz", PWM_FREQ_HZ);

	pwm_dev = DEVICE_DT_GET(DT_NODELABEL(pwm0));
	if (!device_is_ready(pwm_dev)) {
		LOG_ERR("PWM device not ready");
		return 0;
	}

	LOG_INF("PWM device ready: period=%d us", PWM_PERIOD_US);

	while (true) {
		play_audio(pwm_dev);
		k_sleep(K_MSEC(500));
	}

	return 0;
}

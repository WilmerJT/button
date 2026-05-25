/*
 * PWM Audio Playback for nRF5340 - Adapted from nrf52-pwm-audio
 * High-quality audio playback using PWM with precise timing
 * 
 * Key improvement: Uses k_timer for precise sample timing
 * without CPU busy-waiting
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

#include "audio_data.h"

#define PWM_CHANNEL 0
#define PWM_FREQ_HZ 100000
#define PWM_PERIOD_US (1000000 / PWM_FREQ_HZ)
#define PWM_PERIOD_CYCLES 256
// Test helpers
#define TEST_STATIC_PWM_MS 2000
#define TEST_PWM_DUTY_PERCENT 75
#define TEST_USE_LOW_SAMPLE_RATE 0
#define TEST_SAMPLE_RATE_HZ 8000

// Audio playback state machine
typedef struct {
	const uint8_t *data;
	uint32_t length;
	uint32_t index;
	bool playing;
	uint32_t sample_rate;
} audio_playback_t;

static audio_playback_t playback = {0};
static const struct device *pwm_dev;

/*
 * Audio timer callback - fires at sample rate frequency
 * This is called every (1/sample_rate) seconds to update PWM duty cycle
 */
static void audio_sample_callback(struct k_timer *timer)
{
	if (!playback.playing || playback.index >= playback.length) {
		playback.playing = false;
		// Set to neutral/middle PWM (50%)
		pwm_set_cycles(pwm_dev, PWM_CHANNEL, PWM_PERIOD_CYCLES, PWM_PERIOD_CYCLES / 2, PWM_POLARITY_NORMAL);
		return;
	}

	// Get next audio sample (0-255)
	uint8_t sample = playback.data[playback.index];
	
	// Convert to PWM duty cycle
	// 0x00 = silence (0% duty)
	// 0x80 = neutral (50% duty) 
	// 0xFF = maximum (100% duty)
	uint32_t pulse_cycles = sample;
	
	// Clamp to valid range
	if (pulse_cycles > PWM_PERIOD_CYCLES) {
		pulse_cycles = PWM_PERIOD_CYCLES;
	}
	
	// Update PWM output with 8-bit resolution
	pwm_set_cycles(pwm_dev, PWM_CHANNEL, PWM_PERIOD_CYCLES, pulse_cycles, PWM_POLARITY_NORMAL);
	
	playback.index++;
}

K_TIMER_DEFINE(audio_timer, audio_sample_callback, NULL);

/*
 * Initialize PWM audio playback
 */
static int pwm_audio_init(void)
{
	pwm_dev = DEVICE_DT_GET(DT_NODELABEL(pwm0));
	if (!device_is_ready(pwm_dev)) {
		LOG_ERR("PWM device not ready!");
		return -1;
	}

	// Set PWM to idle state (50% duty = silence)
	pwm_set_cycles(pwm_dev, PWM_CHANNEL, PWM_PERIOD_CYCLES, PWM_PERIOD_CYCLES / 2, PWM_POLARITY_NORMAL);
	
	LOG_INF("PWM audio initialized on P0.4");
	LOG_INF("PWM Frequency: %d Hz", PWM_FREQ_HZ);
	
	return 0;
}

/*
 * Start audio playback
 * 
 * @param data - pointer to audio sample data (8-bit unsigned)
 * @param length - number of samples
 * @param sample_rate - playback sample rate (e.g., 22050 Hz)
 * 
 * Returns: 0 if success, -1 if already playing
 */
static int pwm_audio_play(const uint8_t *data, uint32_t length, uint32_t sample_rate)
{
	if (playback.playing) {
		LOG_WRN("Audio already playing!");
		return -1;
	}

	if (!data || length == 0) {
		LOG_ERR("Invalid audio data!");
		return -1;
	}

	playback.data = data;
	playback.length = length;
	playback.index = 0;
	playback.sample_rate = sample_rate;
	playback.playing = true;

	LOG_INF("Starting playback: %d samples @ %d Hz (duration ~%d ms)", 
		length, sample_rate, (length * 1000) / sample_rate);

	// Calculate timer period
	uint32_t period_us = 1000000 / sample_rate;
	
	// Start timer with periodic callback
	k_timer_start(&audio_timer, K_USEC(period_us), K_USEC(period_us));

	return 0;
}

/*
 * Stop audio playback
 */
static void pwm_audio_stop(void)
{
	if (playback.playing) {
		LOG_INF("Stopping audio playback");
		playback.playing = false;
		k_timer_stop(&audio_timer);
		// Set to neutral (silence)
		pwm_set_cycles(pwm_dev, PWM_CHANNEL, PWM_PERIOD_CYCLES, PWM_PERIOD_CYCLES / 2, PWM_POLARITY_NORMAL);
	}
}

__attribute__((unused)) static void pwm_audio_stop_unused(void)
{
	pwm_audio_stop();
}

/*
 * Check if audio is currently playing
 */
static bool pwm_audio_is_playing(void)
{
	return playback.playing;
}

/*
 * Main application
 */
int main(void)
{
	LOG_INF("===== nRF5340 PWM Audio Playback =====");
	LOG_INF("Adapted from nrf52-pwm-audio project");
	LOG_INF("Pin: P0.4");

	// Initialize PWM
	if (pwm_audio_init() != 0) {
		LOG_ERR("Failed to initialize PWM!");
		return -1;
	}

	// Wait a bit before starting playback
	k_sleep(K_MSEC(500));

#if TEST_STATIC_PWM_MS > 0
	// Quick static PWM test: set duty to TEST_PWM_DUTY_PERCENT for TEST_STATIC_PWM_MS ms
	{
		uint32_t duty = (PWM_PERIOD_CYCLES * TEST_PWM_DUTY_PERCENT) / 100;
		LOG_INF("Static PWM test: %d%% for %d ms", TEST_PWM_DUTY_PERCENT, TEST_STATIC_PWM_MS);
		pwm_set_cycles(pwm_dev, PWM_CHANNEL, PWM_PERIOD_CYCLES, duty, PWM_POLARITY_NORMAL);
		k_sleep(K_MSEC(TEST_STATIC_PWM_MS));
		// restore neutral
		pwm_set_cycles(pwm_dev, PWM_CHANNEL, PWM_PERIOD_CYCLES, PWM_PERIOD_CYCLES / 2, PWM_POLARITY_NORMAL);
	}
#endif

	// Play audio in loop
	while (true) {
		if (audio_samples_len > 0) {
			LOG_INF("Playing audio...");
#if TEST_USE_LOW_SAMPLE_RATE
			pwm_audio_play(audio_samples, audio_samples_len, TEST_SAMPLE_RATE_HZ);
#else
			pwm_audio_play(audio_samples, audio_samples_len, audio_sample_rate);
#endif
			
			// Wait for playback to complete
			while (pwm_audio_is_playing()) {
				k_sleep(K_MSEC(50));
			}
			
			LOG_INF("Playback finished");
		} else {
			LOG_WRN("No audio data available!");
		}

		// Pause before next playback
		k_sleep(K_MSEC(2000));
	}

	return 0;
}

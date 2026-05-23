#!/usr/bin/env python3
"""Generate MAXIMUM amplitude test tone"""

import wave
import struct
import math

sample_rate = 22050
duration = 3  # 3 seconds
frequency = 1000
amplitude = 32767  # Maximum for 16-bit signed

output_file = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\wav\test_tone.wav'

print(f"Generating MAXIMUM amplitude test tone...")

num_frames = sample_rate * duration
samples = []

for i in range(num_frames):
    sample = amplitude * math.sin(2 * math.pi * frequency * i / sample_rate)
    samples.append(int(sample))

with wave.open(output_file, 'wb') as wav_file:
    wav_file.setnchannels(1)
    wav_file.setsampwidth(2)
    wav_file.setframerate(sample_rate)
    
    for sample in samples:
        wav_file.writeframes(struct.pack('<h', sample))

print(f"✓ Generated test tone")

# Convert to C with AGGRESSIVE amplification
with wave.open(output_file, 'rb') as wav:
    params = wav.getparams()
    frames = wav.readframes(params.nframes)

fmt = f'<{params.nframes}h'
samples_16bit = struct.unpack(fmt, frames)

# AGGRESSIVE mapping: use full 0-255 range
samples_8bit = []
for s in samples_16bit:
    # Map -32768 to +32767 directly to 0-255
    normalized = int((s + 32768) / 256)
    if normalized > 255:
        normalized = 255
    if normalized < 0:
        normalized = 0
    samples_8bit.append(normalized)

print(f"8-bit range: {min(samples_8bit)} to {max(samples_8bit)}")

c_code = f'''// MAXIMUM amplitude test tone - 1kHz sine wave
#include <stdint.h>

const uint8_t audio_samples[] = {{
'''

for i in range(0, len(samples_8bit), 16):
    chunk = samples_8bit[i:i+16]
    line = "    " + ", ".join(f"0x{s:02x}" for s in chunk)
    if i + 16 < len(samples_8bit):
        line += ","
    c_code += line + "\n"

c_code += f'''
}};

const uint32_t audio_samples_len = {len(samples_8bit)};
const uint32_t audio_sample_rate = {params.framerate};
'''

audio_data_file = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\audio_data.h'
with open(audio_data_file, 'w') as f:
    f.write(c_code)

print(f"✓ Saved audio_data.h with MAXIMUM amplitude!")
print(f"Ready to compile and test.")

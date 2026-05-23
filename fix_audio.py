#!/usr/bin/env python3
import wave
import struct

wav_file = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\wav\sfx-animal-sheep.wav'
output_file = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\audio_data.h'
TARGET_SAMPLE_RATE = 22050

print("Leyendo WAV...")
with wave.open(wav_file, 'rb') as wav:
    params = wav.getparams()
    frames = wav.readframes(params.nframes)

print(f"Canales: {params.nchannels}")
print(f"Sample width: {params.sampwidth}")
print(f"Framerate: {params.framerate} Hz")
print(f"Frames: {params.nframes}")

# Convertir a muestras según sampwidth
if params.sampwidth == 2:
    fmt = f'<{params.nframes * params.nchannels}h'
    samples = list(struct.unpack(fmt, frames))
elif params.sampwidth == 1:
    # 8-bit WAV es unsigned, centrar en 0
    samples = [b - 128 for b in frames]
else:
    raise ValueError(f"sampwidth {params.sampwidth} no soportado")

# Stereo → mono
if params.nchannels == 2:
    mono = [(samples[i] + samples[i+1]) // 2 for i in range(0, len(samples), 2)]
else:
    mono = samples

print(f"Muestras mono: {len(mono)}")
print(f"Rango original: {min(mono)} a {max(mono)}")

# Resample correcto (funciona para up y downsample)
original_rate = params.framerate
if original_rate != TARGET_SAMPLE_RATE:
    # Resample por interpolación lineal
    duration = len(mono) / original_rate
    num_samples_out = int(duration * TARGET_SAMPLE_RATE)
    resampled = []
    for i in range(num_samples_out):
        src_pos = i * original_rate / TARGET_SAMPLE_RATE
        idx = int(src_pos)
        frac = src_pos - idx
        if idx + 1 < len(mono):
            val = mono[idx] * (1 - frac) + mono[idx + 1] * frac
        else:
            val = mono[idx]
        resampled.append(val)
    mono = resampled
    print(f"Resample: {original_rate} Hz → {TARGET_SAMPLE_RATE} Hz")
    print(f"Muestras después de resample: {len(mono)}")
else:
    print("Sin resample necesario")

# Normalizar a 8-bit unsigned (0-255) para PWM
min_val = min(mono)
max_val = max(mono)
range_val = max_val - min_val

samples_8bit = []
for s in mono:
    normalized = int(((s - min_val) * 255) / range_val) if range_val > 0 else 128
    samples_8bit.append(max(0, min(255, normalized)))

print(f"Rango 8-bit: {min(samples_8bit)} a {max(samples_8bit)}")
print(f"Duración final: {len(samples_8bit) / TARGET_SAMPLE_RATE:.3f} s")

# Generar header C
c_code = f'''// Audio samples - auto-generated from WAV
// Source: {wav_file}
// Original rate: {original_rate} Hz -> Resampled: {TARGET_SAMPLE_RATE} Hz
// Duration: {len(samples_8bit) / TARGET_SAMPLE_RATE:.3f} seconds

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
const uint32_t audio_sample_rate = {TARGET_SAMPLE_RATE};
'''

with open(output_file, 'w', encoding='utf-8') as f:
    f.write(c_code)

print(f"\n✓ Guardado en: {output_file}")
print(f"  Muestras: {len(samples_8bit)}")
print(f"  Duración: {len(samples_8bit) / TARGET_SAMPLE_RATE:.3f} s")
print(f"  Tamaño aprox: {len(samples_8bit) / 1024:.1f} KB")
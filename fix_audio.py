#!/usr/bin/env python3
import wave
import struct

wav_file = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\wav\sfx-animal-sheep.wav'
output_file = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\audio_data.h'

print("Leyendo WAV...")
with wave.open(wav_file, 'rb') as wav:
    params = wav.getparams()
    frames = wav.readframes(params.nframes)

print(f"Canales: {params.nchannels}")
print(f"Sample width: {params.sampwidth}")
print(f"Framerate: {params.framerate} Hz")
print(f"Frames: {params.nframes}")

# Convertir a muestras 16-bit con signo
if params.sampwidth == 2:
    fmt = f'<{params.nframes * params.nchannels}h'
    samples = struct.unpack(fmt, frames)
else:
    samples = list(frames)

print(f"Muestras leídas: {len(samples)}")

# Si estéreo, convertir a mono
if params.nchannels == 2:
    mono = [(samples[i] + samples[i+1]) // 2 for i in range(0, len(samples), 2)]
else:
    mono = list(samples)

print(f"Muestras mono: {len(mono)}")
print(f"Rango: {min(mono)} a {max(mono)}")

# Normalizar a 8-bit (0-255) para PWM
# Rango de -32768 a +32767 → 0 a 255
min_val = min(mono)
max_val = max(mono)
range_val = max_val - min_val

samples_8bit = []
for s in mono:
    normalized = int(((s - min_val) * 255) / range_val) if range_val > 0 else 128
    samples_8bit.append(normalized)

print(f"Rango 8-bit: {min(samples_8bit)} a {max(samples_8bit)}")

# Generar código C
c_code = f'''// Audio samples - auto-generated from WAV
// Source: {wav_file}
// Framerate: {params.framerate} Hz, Channels: {params.nchannels}
// Duration: {len(samples_8bit) / params.framerate:.2f} seconds

#include <stdint.h>

const uint8_t audio_samples[] = {{
'''

# Escribir en líneas de 16 valores
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

# Guardar
with open(output_file, 'w') as f:
    f.write(c_code)

print(f"\n✓ Guardado en: {output_file}")
print(f"  Muestras: {len(samples_8bit)}")
print(f"  Duración: {len(samples_8bit) / params.framerate:.2f}s")
print(f"  Tamaño: {len(samples_8bit)} bytes")

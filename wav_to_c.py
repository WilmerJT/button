#!/usr/bin/env python3
"""
Convierte un archivo WAV a un array C para incrustación de audio.
"""

import wave
import struct
import sys
from pathlib import Path

def wav_to_c(wav_path, output_path, downsample=1):
    """Convierte WAV a array C."""
    
    with wave.open(wav_path, 'rb') as wav:
        params = wav.getparams()
        frames = wav.readframes(params.nframes)
    
    # Parámetros WAV
    n_channels = params.nchannels
    sample_width = params.sampwidth
    sample_rate = params.framerate
    n_frames = params.nframes
    
    print(f"WAV Info:")
    print(f"  Canales: {n_channels}")
    print(f"  Ancho: {sample_width} bytes")
    print(f"  Frecuencia: {sample_rate} Hz")
    print(f"  Muestras: {n_frames}")
    print(f"  Duración: {n_frames / sample_rate:.2f}s")
    
    # Convertir a muestras (16-bit)
    samples = []
    for i in range(0, len(frames), sample_width * n_channels):
        if sample_width == 2:
            sample = struct.unpack('<h', frames[i:i+2])[0]
        else:
            sample = frames[i] - 128
        samples.append(sample)
    
    # Downsample si es necesario
    if downsample > 1:
        samples = samples[::downsample]
    
    # Normalizar a 8-bit (0-255) para PWM duty cycle
    max_sample = max(abs(s) for s in samples)
    if max_sample > 0:
        samples_8bit = [(((s + max_sample) * 255) // (2 * max_sample)) for s in samples]
    else:
        samples_8bit = [128] * len(samples)
    
    # Generar código C
    c_code = f"""// Audio data - {Path(wav_path).name}
// Generado automáticamente desde WAV
// Frecuencia de muestreo original: {sample_rate} Hz
// Frecuencia de muestreo final: {sample_rate // downsample} Hz
// Duración: {len(samples_8bit) / (sample_rate // downsample):.2f}s

#include <stdint.h>

const uint8_t audio_samples[] = {{
"""
    
    # Escribir muestras en grupos de 16 por línea
    for i in range(0, len(samples_8bit), 16):
        chunk = samples_8bit[i:i+16]
        line = "    " + ", ".join(f"0x{s:02x}" for s in chunk)
        if i + 16 < len(samples_8bit):
            line += ","
        else:
            line += ""
        c_code += line + "\n"
    
    c_code += f"""}};

const uint32_t audio_samples_len = {len(samples_8bit)};
const uint32_t audio_sample_rate = {sample_rate // downsample};
"""
    
    with open(output_path, 'w') as f:
        f.write(c_code)
    
    print(f"\nGenerado: {output_path}")
    print(f"Muestras: {len(samples_8bit)}")
    print(f"Tamaño: {len(samples_8bit)} bytes")

if __name__ == '__main__':
    wav_path = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\wav\sfx-animal-sheep.wav'
    output_path = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\audio_data.h'
    
    # Sin downsample
    wav_to_c(wav_path, output_path, downsample=1)

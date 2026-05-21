import wave
import struct

wav_file = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\wav\sfx-animal-sheep.wav'

try:
    with wave.open(wav_file, 'rb') as wav:
        params = wav.getparams()
        frames = wav.readframes(params.nframes)
    
    print(f"✓ WAV cargado: {len(frames)} bytes")
    print(f"  Canales: {params.nchannels}")
    print(f"  Sample width: {params.sampwidth}")
    print(f"  Framerate: {params.framerate} Hz")
    print(f"  Frames: {params.nframes}")
    
    # Convertir a muestras 16-bit
    samples = struct.unpack(f'<{params.nframes * params.nchannels}h', frames)
    print(f"  Muestras convertidas: {len(samples)}")
    
    # Si estéreo, promediar a mono
    if params.nchannels == 2:
        mono_samples = [(samples[i] + samples[i+1]) // 2 for i in range(0, len(samples), 2)]
    else:
        mono_samples = list(samples)
    
    # Normalizar a 8-bit (0-255) para PWM duty cycle
    min_sample = min(mono_samples)
    max_sample = max(mono_samples)
    range_sample = max_sample - min_sample if max_sample > min_sample else 1
    
    samples_8bit = [int(((s - min_sample) * 255) / range_sample) for s in mono_samples]
    
    print(f"  Rango original: {min_sample} a {max_sample}")
    print(f"  Convertido a 8-bit: {min(samples_8bit)} a {max(samples_8bit)}")
    
    # Generar archivo C
    c_code = f'''// Audio samples - auto-generated from WAV
// {wav_file}
// Framerate: {params.framerate} Hz, Channels: {params.nchannels}

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
    
    output = r'C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n\src\audio_data.h'
    with open(output, 'w') as f:
        f.write(c_code)
    
    print(f"\n✓ Guardado: {output}")
    print(f"  {len(samples_8bit)} muestras de audio")
    print(f"  {len(samples_8bit) / params.framerate:.2f} segundos de duración")

except Exception as e:
    print(f"❌ Error: {e}")
    import traceback
    traceback.print_exc()

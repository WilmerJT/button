# Audio Playback via PWM para LM386N (NRF5340)

## Configuración

### Hardware
- **Board**: NRF5340 DK
- **Pin PWM**: P1.11 (GPIO 1 Pin 11)
- **Amplificador**: LM386N
- **Frecuencia PWM**: 100 kHz
- **Voltaje**: 3.3V (NRF5340) → filtro RC → LM386N

### Circuito Recomendado
```
NRF5340 (P1.11)
     ↓
    [10K resistor]
     ↓
    [PWM signal] → [10µF capacitor] → GND
     ↓
  [LM386N audio input]
```

## Cómo Funciona

1. **audio_data.h** contiene las muestras de audio en formato 8-bit (0-255)
2. El código PWM convierte cada muestra a un duty cycle
3. El filtro RC suaviza la onda PWM → señal analógica
4. El LM386N amplifica la señal

## Generar Datos de Audio desde WAV

### Opción 1: Usar el script Python (recomendado)

```bash
# Ejecutar el script de conversión
python wav_to_c.py

# Reemplaza el contenido de src/audio_data.h
# con el contenido generado
```

El script:
- Lee el WAV (16-bit, mono o estéreo)
- Normaliza a 8-bit (0-255)
- Genera un archivo C con el array de muestras

### Opción 2: Desde PowerShell

```powershell
$wavPath = "src\wav\sfx-animal-sheep.wav"
python wav_to_c.py
```

## Parámetros Ajustables

En `src/main.c`:
- `PWM_FREQ_HZ`: Frecuencia del PWM (100 kHz = buen balance audio-filtro)
- `audio_sample_rate`: En `audio_data.h`, define la velocidad de reproducción

Para mejor calidad de audio:
- Aumentar `PWM_FREQ_HZ` (ej: 200 kHz) para mejor filtrado
- Usar sample rate de 16-44 kHz en el WAV original

## Compilar y Flashear

```bash
west build -b nrf5340dk_nrf5340_cpuapp
west flash
```

## Monitoreo

```bash
# Terminal serial a 115200 baud
screen /dev/ttyUSB0 115200
```

## Notas

- El audio actual es una **demo con 16 muestras** - generar desde tu WAV
- Para MP3: convertir MP3 → WAV primero (ej: ffmpeg)
- Ajustar valores de resistor/capacitor según impedancia del LM386N
- Si no hay sonido: verificar conexión P1.11 y filtro RC

## Troubleshooting

| Problema | Solución |
|----------|----------|
| PWM device not ready | Verificar overlay del device tree |
| Sin sonido | Revisar conexión del pin, filtro RC |
| Distorsión | Reducir volumen LM386N o mejorar filtro |
| Ruido | Aumentar frecuencia PWM a 200+ kHz |


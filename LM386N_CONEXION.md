# Conexión LM386N con nRF5340 - PWM a Audio

## Esquema de Conexión

```
┌─────────────────────────────────────────────────────────────┐
│                      nRF5340 DK                             │
│                                                             │
│  P1.11 (PWM) ────────────┐                                 │
│                          │                                 │
└──────────────────────────┼─────────────────────────────────┘
                           │
                    [Filtro RC]
                    10kΩ resistor
                           │
                    ┌──────┴──────┐
                    │             │
                   (R)           (C)
                    │        10µF cap
                    │             │
                    │            GND
                    │
            ┌───────┴────────┐
            │                │
         [10µF cap]         │
            │                │
            ├───(pin 3)──────┤  LM386N
            │ IN+ (non-inv)  │
            │                │
         [100k Ω]           │  pin 1: GND
            │                │  pin 2: IN- (100k to GND + 10µF cap)
            │                │  pin 3: IN+ (filtered PWM)
            GND              │  pin 4: GND
                             │  pin 5: OUT (audio)
                             │  pin 6: +V (4-12V)
                             │  pin 7: GND
                             │  pin 8: GAIN (100k between pins 1-8)
                             │
            ┌────────────────┤
            │              pin 2 (IN-)
         [100k Ω]
            │
           GND    ┌────────[10µF]────────┐
                  │                      │
                  │ (pin 5) OUT  ──────┬─┴──┐
                  │                    │    │
                  │              [8Ω speaker]
                  │                    │
               GND ────────────────────┴────┘
```

## Detalles de Conexión

### Pines LM386N (DIP-8):
```
    ┌─────────┐
    │ 1    8  │
    │ 2    7  │
    │ 3    6  │
    │ 4    5  │
    └─────────┘

1: GAIN- / GND
2: IN- (entrada no inversora)
3: IN+ (entrada inversora) 
4: GND
5: OUT (salida)
6: VCC (+5V o más)
7: BYPASS
8: GAIN+
```

### Conexiones Específicas:

| Componente | Conexión |
|-----------|----------|
| **nRF5340 P1.11** | → [10kΩ resistor] → LM386N pin 3 |
| **10µF capacitor** | Entre P1.11 y GND (filtro PWM) |
| **LM386N pin 1** | GND |
| **LM386N pin 2** | → [100kΩ a GND] + [10µF capacitor] |
| **LM386N pin 3** | PWM filtrado (desde nRF5340) |
| **LM386N pin 4** | GND |
| **LM386N pin 5** | → [8Ω altavoz] → GND |
| **LM386N pin 6** | +5V (o 4-12V) |
| **LM386N pin 7** | GND |
| **LM386N pin 8** | → [100kΩ] → LM386N pin 1 (ganancia x100) |

### Alimentación:
- **LM386N VCC (pin 6)**: +5V (mínimo 4V, máximo 12V)
- Añadir capacitor de bypass de 10µF entre VCC y GND

---

## Diagrama Simplificado (texto):

```
nRF5340                      LM386N (DIP-8)              Altavoz
┌────────┐                   ┌──────────┐
│ P1.11  │─[10kΩ]─┬─[10µF]─┬│ 3 IN+    │
│(PWM)   │        │         ││          │
└────────┘        │         ││ 2 IN-    │─[100kΩ]─┐
                  │         ││          │         GND
                  │         ││ 1 GAIN-  │
                  │       GND│ 4 GND    │
                  │          │ 6 VCC    │─[+5V]
                  │          │ 5 OUT    │────[8Ω Speaker]
                  │          │ 8 GAIN+  │
                  │          └──────────┘
                  │                │
                  └────[10µF]──┬───┴────┐
                             GND     GND
```

---

## ⚠️ Componentes Necesarios:

- 1x LM386N (DIP-8)
- 2x Capacitor 10µF (electrolítico)
- 2x Resistor 100kΩ
- 1x Resistor 10kΩ
- 1x Altavoz 8Ω (0.5-2W)
- Fuente de alimentación 5V

---

## 📍 Notas Importantes:

1. **Ganancia**: Con 100kΩ entre pines 1 y 8 → ganancia x100
   - Para ganancia x20: dejar pines 1-8 sin conectar
   - Para ganancia variable: usar potenciómetro 100kΩ

2. **Filtro RC**: 10kΩ + 10µF = corte ~1.6kHz (suaviza PWM)
   - Aumentar C si quieres más atenuación de ruido PWM
   - Reducir si necesitas más agudos

3. **Bypass**: Añadir 100nF + 10µF en VCC si hay ruido

4. **Impedancia**: LM386N tiene baja impedancia de salida (~75Ω)
   - Compatible con altavoces 4-16Ω

5. **Voltaje**: Funciona con 4-12V
   - Usa USB (5V) del nRF5340 si disponible
   - O fuente externa 5V

---

## 🔊 Prueba de Sonido:

Una vez flasheado:
1. Conecta todo según el diagrama
2. Dale alimentación al LM386N
3. Deberías escuchar el audio en el altavoz
4. Ajusta el volumen con el potenciómetro de ganancia (si lo usas)


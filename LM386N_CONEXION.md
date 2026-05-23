# Conexión LM386N con nRF5340 - PWM a Audio
## ⭐ MODIFICADO: Aumentar Ganancia del S8050

## Esquema de Conexión (OPTIMIZADO para MÁXIMO VOLUMEN)

```
┌─────────────────────────────────────────────────────────┐
│              nRF5340 P0.4 (PWM)                         │
└────────┬────────────────────────────────────────────────┘
         │
    [10kΩ] ← RESISTOR BASE REDUCIDO de 10kΩ
         │                    ↓ CAMBIO: Usar 1-4.7kΩ para más ganancia
         │
    [10µF capacitor] ← Filtro RC
         │
         ├──────────────────┐ S8050 BJT NPN
         │                  │
         │          BASE ──┤| ← PWM filtrado llega aquí
         │          COLECTOR: a LM386N
         │          EMISOR: a GND
         │
     COLECTOR del S8050
         │
    [1-2.2kΩ] ← RESISTOR COLECTOR reducido (era 10kΩ)
         │         ↓ CAMBIO: Menor R = más corriente
         │
    [1µF capacitor] ← Acople a LM386N pin 3
         │
    [opcional: 100kΩ a GND] ← Resistor pull-down para estabilidad
         │
         └──────────┬──────────────────┐  LM386N
                    │                  │
               (pin 3) IN+ ────────────┤
                                       │
                  (pin 1) GAIN- ────┤  ← GND
                  (pin 2) IN- ───┬──┤
                                 │  │
                           [100kΩ]  │ pin 4: GND
                                 │  │ pin 6: VCC (+5V)
                                GND │ pin 5: OUT
                                    │
                    (pin 8) GAIN+ ──┤
                          ↓         │
                    [100kΩ]─(pin 1) ← Ganancia x100
                          │
                    (pin 5) OUT ─────[10µF cap]─────┐
                                                     │
                                              [8Ω Speaker]
                                                     │
                                                    GND
```


## CAMBIOS REQUERIDOS para MÁXIMO VOLUMEN

### ⚡ Modificación 1: Resistor de Base (10kΩ → 1-4.7kΩ)

**Actual**: 10kΩ = baja corriente de base = ganancia reducida
**Nuevo**: **1kΩ o 4.7kΩ** = más corriente base = S8050 más saturado = máxima ganancia

**Conexión S8050**:
```
                PWM filtrado (P0.4)
                       │
                   [1-4.7kΩ] ← REDUCIR AQUÍ
                       │
                    ┌──│
                    │  B (base)
              S8050 │  E (emisor) ─ GND
                    │  C (colector) ─ a LM386N pin 3
                    └──│
```

**Ganancia aproximada del transistor**:
- Con 10kΩ: Ic/Ib ≈ 20-50x (bajo)
- Con 1kΩ: Ic/Ib ≈ 100-200x (ALTO) ← USAR ESTO

---

### ⚡ Modificación 2: Resistor Colector (10kΩ → 1-2.2kΩ)

**Actual**: 10kΩ en colector = poca caída de voltaje = baja amplitud
**Nuevo**: **1-2.2kΩ** = más caída = máxima amplitud en colector

**Conexión**:
```
            +5V (VCC)
              │
          [1-2.2kΩ] ← RESISTOR COLECTOR
              │
            ┌─┴──
            │   C (colector)
       S8050│
            │   E (emisor)
            └────GND
```

---

### ⚡ Modificación 3: Capacitor de Acople (opcional)

Cambiar de 10µF a **1µF** para mejor respuesta de frecuencia en audio:
```
S8050 Colector ─[1µF capacitor]─ LM386N pin 3
```

---

## 🔧 PASOS PARA MODIFICAR

1. **Desconecta todo** (sin alimentación)
2. **Reemplaza resistor base**:
   - ✅ Si usas 1kΩ: máxima ganancia pero más consumo
   - ✅ Si usas 4.7kΩ: ganancia media, menor consumo
   - **Recomendación: 1kΩ**

3. **Reemplaza resistor colector**:
   - De 10kΩ a **2.2kΩ**

4. **Opcional - cambia capacitor**:
   - De 10µF a **1µF** (más agudos, menos bajos)

5. **Reconecta**:
   ```
   P0.4 → [1kΩ resistor] → S8050 base
   S8050 colector → [1µF cap] → LM386N pin 3
   ```

---

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


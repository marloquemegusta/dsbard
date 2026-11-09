# Samples

Las muestras de sonido del proyecto son síntesis offline propia generada por scripts del proyecto (sin grabaciones ni material con derechos de terceros) y se redistribuyen como CC0-equivalente.

## Formato y Configuración

- **Formato**: PCM lineal mono, 8-bit con signo, 22.050 Hz, binario raw para el mezclador hardware directo (`soundPlaySample`).
- **Multisampling melódico**: 4 raíces MIDI (48, 60, 72 y 84) por cada instrumento melódico PCM, seleccionando la raíz más próxima y recalculando la tasa de muestreo para afinar la nota con exactitud.
- **Volumen uniforme**: 95/127 tanto para preescucha en vivo como para reproducción de bucles en todas las pistas, con compensación suave de `(60 - midi) / 4` para preservar presencia en graves sin saturación.

## Lista de 10 Instrumentos

1. **`BRASS`** (PCM multisampleado, 4 raíces, loop en muestra 14.332)
2. **`STRINGS`** (PCM multisampleado, 4 raíces, loop en muestra 17.640)
3. **`CHOIR`** (PCM multisampleado con síntesis de formantes: vocal profunda tipo *"Ohhh"* en raíces 48 y 60; vocal brillante tipo *"Ahhh"* en raíces 72 y 84; loop en muestra 12.938)
4. **`FLUTE`** (PCM multisampleado, 4 raíces, loop en muestra 14.332)
5. **`PIANO`** (PCM multisampleado Rhodes/acústico, 4 raíces, one-shot sin loop)
6. **`PLUCK`** (PCM multisampleado arpa/cuerda pulsada, 4 raíces, one-shot sin loop)
7. **`BASS`** (PCM multisampleado bajo acústico/eléctrico, 4 raíces, one-shot sin loop)
8. **`CHIP LEAD`** (Generador PSG de onda pulso al 50%, duty cycle `DutyCycle_50`, con envolvente software ADSR suave para evitar clics)
9. **`CHIP WAVE`** (Generador PSG de onda pulso al 12%, duty cycle `DutyCycle_12`, con envolvente software ADSR suave)
10. **`DRUMS`** (Kit de 12 muestras de percusión one-shot asignadas a las 12 teclas cromáticas `0..11`, reproducidas a 22.050 Hz sin loop):
    - `0`: Kick profundo (`drum_0.bin`)
    - `1`: Low Tom cinematográfico (`drum_1.bin`)
    - `2`: Mid Tom cinematográfico (`drum_2.bin`)
    - `3`: High Tom cinematográfico (`drum_3.bin`)
    - `4`: Caja limpia y seca (`drum_4.bin`)
    - `5`: Caja con reverb amplia (`drum_5.bin`)
    - `6`: Rimshot / Palma (`drum_6.bin`)
    - `7`: Tom retro analógico (`drum_7.bin`)
    - `8`: Hi-hat cerrado (`drum_8.bin`)
    - `9`: Hi-hat abierto (`drum_9.bin`)
    - `10`: Crash cymbal orquestal (`drum_10.bin`)
    - `11`: Chime / Campana cinematográfica (`drum_11.bin`)

## Motor de Audio y Hardware DS

Se utiliza el mezclador hardware directo de la Nintendo DS (canales PSG y PCM) mediante `soundPlaySample` y `soundPlayPSG`, manteniendo frecuencias exactas en Hz (y `8 * Hz` para libnds). Esto asegura un presupuesto de CPU mínimo para el audio y máxima respuesta táctil a 60 Hz.

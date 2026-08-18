# DS Stylophone / Looper

Instrumento homebrew para Nintendo DS/DSi: 12 franjas cromáticas táctiles numeradas `0`–`11`, tres loops independientes de eventos musicales y un diario temporal. No guarda datos entre sesiones.

La ROM incluye seis timbres PCM multisampleados (`RHODES`, `KALIMBA`, `BASS`, `PLUCK`, `GLASS`, `PAD`) y cuatro timbres PSG de referencia (`SQUARE`, `PULSE`, `THIN`, `DARK`). Cada nota grabada conserva su `instrument_id`, así que los tres tracks pueden reproducir timbres distintos mientras la voz live usa el timbre actual. El origen y formato PCM están en [SAMPLES.md](SAMPLES.md).

## Compilar

El único build canónico usa BlocksDS dentro de Docker:

```powershell
.\build.ps1
```

Genera la única ROM final `ds-stylophone.nds` en la raíz del proyecto.

Las pruebas de la lógica de loops se ejecutan con `g++ -std=c++17 -Iinclude tests/test_loop_logic.cpp -o tests/test_loop_logic.exe; tests/test_loop_logic.exe`.

Si melonDS está descargado en `.tools/melonds`, `.\run-emulator.ps1` abre la ROM directamente.
`.\run-emulator.ps1` recompila esa misma ROM canónica y la abre en NO$GBA.

Para compilar y subir la ROM canónica a la Nintendo DS por FTP:

```powershell
.\tools\upload-rom-ftp.ps1
```

El script recompila BlocksDS antes de subir y usa el destino `ftp://192.168.1.151:5000/roms/nds/ds-stylophone.nds`.

## Controles

- Touchscreen: tocar y arrastrar las franjas numéricas `0`–`11`.
- D-pad arriba/abajo: octava 2–6.
- D-pad izquierda/derecha: seleccionar Track 1–3.
- A: grabar/detener grabación de la pista seleccionada.
- B corto: borrar el último token del diario; B mantenido: limpiar el diario.
- Y sin tocar: insertar separador de frase.
- L+B: deshacer la última regrabación; R+B: limpiar la pista seleccionada.
- START: pausar/reanudar el transporte.
- SELECT corto: mute/unmute de la pista; SELECT mantenido: ayuda temporal.
- X: cambiar timbre (`RHODES` → `KALIMBA` → `BASS` → `PLUCK` → `GLASS` → `PAD` → `SQUARE` → `PULSE` → `THIN` → `DARK`); L sustain; L+izquierda vibrato; L+derecha glide entre notas.

El diario transcribe literalmente las franjas pulsadas como valores `0`–`11`. No tiene en cuenta octava, MIDI ni frecuencia. Muestra los tokens más recientes y no registra las notas producidas por los loops. La interfaz no muestra nombres de notas: el audio conserva internamente la representación MIDI.

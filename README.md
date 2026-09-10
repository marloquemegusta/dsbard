# DS Stylophone / Looper

Instrumento homebrew para Nintendo DS/DSi: 12 franjas cromáticas táctiles numeradas `0`–`11`, cuatro loops independientes de eventos musicales y un diario temporal. Al iniciar permite continuar un tema guardado, crear uno nuevo o elegirlo desde una biblioteca de hasta 16 temas; los cambios se guardan automáticamente.

La ROM incluye seis timbres PCM multisampleados (`RHODES`, `KALIMBA`, `BASS`, `PLUCK`, `GLASS`, `PAD`) y cuatro timbres PSG de referencia (`SQUARE`, `PULSE`, `THIN`, `DARK`). Cada nota grabada conserva su `instrument_id`, así que los cuatro tracks pueden reproducir timbres distintos mientras la voz live usa el timbre actual. El origen y formato PCM están en [SAMPLES.md](SAMPLES.md).

## Compilar

El único build canónico usa BlocksDS dentro de Docker:

```powershell
.\build.ps1
```

Genera la única ROM final `ds-stylophone.nds` en la raíz del proyecto.

Para generar capturas PNG nativas de NO$GBA para el manual o una revisión visual, consulta [docs/nogba-captures.md](docs/nogba-captures.md).
Para automatizar secuencias de checkpoints UX, consulta [docs/nogba-ux-scenarios.md](docs/nogba-ux-scenarios.md).

Las pruebas de la lógica de loops se ejecutan con `g++ -std=c++17 -Iinclude tests/test_loop_logic.cpp -o tests/test_loop_logic.exe; tests/test_loop_logic.exe`.

Si melonDS está descargado en `.tools/melonds`, `.\run-emulator.ps1` abre la ROM directamente.
`.\run-emulator.ps1` recompila esa misma ROM canónica y la abre en NO$GBA.

Para compilar y subir la ROM canónica a la Nintendo DS por FTP:

```powershell
.\tools\upload-rom-ftp.ps1
```

El script recompila BlocksDS antes de subir y usa el destino `ftp://192.168.1.151:5000/roms/nds/ds-stylophone.nds`.

## Controles

La franja inferior muestra los controles compactos `PULSE`, vibrato y sustain en modo normal. Mantén `SELECT` para mover literalmente el monitor de pistas a la pantalla táctil: el teclado queda arriba y abajo se ve la misma pantalla de pistas, con sus cajas, diario y controles. Toca una caja para abrir el mismo editor de notas de la aceptación; no hay playhead ni selección global en esta vista.

- Touchscreen: tocar y arrastrar las franjas numéricas `0`–`11`.
- D-pad arriba/abajo: octava 2–6.
- D-pad izquierda/derecha: seleccionar Track 1–4.
- A: grabar/detener grabación de la pista seleccionada.
- B corto: borrar el último token del diario; B mantenido: limpiar el diario.
- Y: activar/desactivar loop; el loop respeta la selección temporal actual o toda la canción si no hay selección.
- L+B: deshacer la última regrabación; R+B: limpiar la pista seleccionada.
- START: pausar/reanudar; con loop desactivado, el transporte se detiene al final y vuelve a arrancar desde la ventana al pulsar START.
- SELECT corto: mute/unmute de la pista; SELECT mantenido: abrir/cerrar el selector de frases.
- X: cambiar timbre (`RHODES` → `KALIMBA` → `BASS` → `PLUCK` → `GLASS` → `PAD` → `SQUARE` → `PULSE` → `THIN` → `DARK`); L sustain; en aceptación, L corto alterna el sustain de la nota seleccionada; franja inferior PULSE/VIB/SUS para configurar pulso y efectos.
- Inicio: `CONTINUE` abre el último tema disponible, `NEW` crea uno nuevo y `SONGS` muestra la biblioteca. Los nombres se dibujan con el stylus y se guardan junto al tema.

El diario transcribe literalmente las franjas pulsadas como valores `0`–`11`. No tiene en cuenta octava, MIDI ni frecuencia. Muestra los tokens más recientes y no registra las notas producidas por los loops. La interfaz no muestra nombres de notas: el audio conserva internamente la representación MIDI.

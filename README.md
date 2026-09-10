# DS Stylophone / Looper

Instrumento homebrew para Nintendo DS/DSi: 12 franjas cromáticas táctiles numeradas `0`–`11`, cuatro loops independientes de eventos musicales y un diario temporal. Al iniciar permite continuar un tema guardado, crear uno nuevo o elegirlo desde una biblioteca de hasta 16 temas; los cambios se guardan automáticamente.

La ROM incluye seis timbres PCM multisampleados (`RHODES`, `KALIMBA`, `BASS`, `PLUCK`, `GLASS`, `PAD`) y cuatro timbres PSG de referencia (`SQUARE`, `PULSE`, `THIN`, `DARK`). Cada nota grabada conserva su `instrument_id`, así que los cuatro tracks pueden reproducir timbres distintos mientras la voz live usa el timbre actual. El origen y formato PCM están en [SAMPLES.md](SAMPLES.md).

## Contexto de recuperación

Este repositorio está en un **punto intermedio de reconstrucción**, no es una recuperación exacta de la fuente original. La carpeta local donde se había desarrollado la versión más avanzada se borró accidentalmente desde la terminal y no pudo recuperarse desde la papelera. La copia de OneDrive disponible era anterior y no contenía todos los cambios recientes.

La ROM que estaba instalada en la Nintendo DS sí se conservó. Se extrajo y se guarda localmente como `recovered/ds-stylophone-from-ds.nds`. Esa es la ROM buena conocida: sirve para comparar interfaz, comportamiento y, cuando sea posible, instrumentos, pero una ROM compilada no sustituye al código fuente perdido.

El estado que se mantiene en `main` intenta acercarse a esa ROM buena a partir de la copia de seguridad antigua, la ROM extraída de la DS, las conversaciones de desarrollo y los artefactos de pruebas recuperados. La ROM que se construye actualmente es `ds-stylophone.nds`; representa el estado actual de la reconstrucción y **no debe considerarse idéntica todavía** a la ROM original. El trabajo futuro debe continuar desde este punto comparando ambas ROMs y validando especialmente la interfaz y el comportamiento en hardware.

Referencias binarias actuales:

- ROM buena extraída de la DS: `recovered/ds-stylophone-from-ds.nds` — SHA-256 `e826ecde2da3a1413d874ae39772d1d6389f7311f1ebc95a79f447d6b14d30b7`.
- ROM actual reconstruida y canónica del repositorio: `ds-stylophone.nds` — SHA-256 `7235d5b6c32ce794f386144fa1e296122ee575522fe1df2572a7740666db669d`.
- `recovered/ftp-verify.nds` es una copia de verificación de una transferencia anterior; no es la fuente de verdad.

Las dos ROMs importantes también están publicadas como assets de la release [reconstruction-baseline-2026-09-10](https://github.com/marloquemegusta/dsbard/releases/tag/reconstruction-baseline-2026-09-10). El detalle de procedencia, nombres y hashes está en [docs/ROM_ARTIFACTS.md](docs/ROM_ARTIFACTS.md).

Mientras continúe la reconstrucción, no se debe sobrescribir la ROM de referencia de `recovered/` ni asumir que un build verde demuestra equivalencia con la versión perdida. `main` es deliberadamente la base versionada desde la que retomar ese esfuerzo.

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

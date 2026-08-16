# AGENTS.md — DS Stylophone / Looper

## Regla principal de ejecución y pruebas

- Antes de probar una ROM, comprobar si hay un dispositivo Android conectado y autorizado por ADB:
  `C:\Users\malfonso\AppData\Local\Android\Sdk\platform-tools\adb.exe devices -l`
- Si hay un dispositivo disponible, usar siempre el melonDS Android conectado por ADB para cargar y probar la ROM. No lanzar el melonDS de Windows salvo que no haya ningún dispositivo ADB disponible.
- La ROM de prueba del dispositivo es:
  `/sdcard/ROMs/Nds/ds-stylophone.nds`
- Flujo de despliegue:
  1. Compilar con `build.ps1`.
  2. Sustituir la ROM remota con `adb push .\ds-stylophone.nds /sdcard/ROMs/Nds/ds-stylophone.nds`.
  3. Reiniciar melonDS Android (`adb shell am force-stop me.magnum.melonds`) y abrirlo de nuevo.
  4. Confirmar que está activo `me.magnum.melonds/.ui.emulator.EmulatorActivity`.
- Si hay varias pantallas Android, para capturas usar el display ID real obtenido con `adb shell dumpsys SurfaceFlinger --display-id`; no asumir que `screencap` usa la pantalla correcta por defecto.

## Lecciones técnicas del proyecto

- Las 12 zonas táctiles son cromáticas, de anchura idéntica y en este orden exacto: `C | C# | D | D# | E | F | F# | G | G# | A | A# | B`.
- El índice táctil y el dibujo deben usar la misma fórmula: `key = (x * 12) / 256`, con `x` calibrada en el rango `0..255`.
- La correspondencia de tono debe probarse completa y también específicamente en los límites problemáticos: `E4=329 Hz`, `F4=349 Hz` y `B4 < C5`.
- No usar una redibujado completo de la bitmap directamente en VRAM en cada frame. Renderizar primero en un framebuffer de RAM y copiar a VRAM; el repintado incremental solo debe copiar las franjas que cambian.
- Las etiquetas de notas deben dibujarse explícitamente para las 12 zonas y verificarse visualmente en melonDS Android. No dar por válida una captura donde falte cualquier etiqueta.
- `soundSetFreq` debe reservarse para glide y vibrato. Las notas discretas deben arrancar con `soundPlayPSG` usando la frecuencia correcta, para evitar escrituras continuas del temporizador PSG y transitorios engañosos.
- No asumir que una fórmula matemáticamente correcta basta: comprobar la API instalada de libnds, en especial que `soundPlayPSG` y `soundSetFreq` reciben frecuencia en Hz.
- Al diagnosticar audio, mostrar temporalmente en pantalla la nota activa, MIDI y Hz; esto separa errores de mapeo de problemas de reproducción o de la mezcla con loops.

## Registro de bloqueos resueltos

### Lecciones de la pasada Android del 2026-08-16

- La pantalla superior debe mostrar `REC/PLAY/MUTE`, actividad de la toma provisional, timbre real, sustain y vibrato.
- La toma temporal vive en `recording_target` hasta terminar la grabaciÃ³n; durante `REC` el monitor debe dibujar ese destino y no solo `tracks[]`.
- El overlay de melonDS Android puede interceptar los toques. Una captura con un icono del overlay activo no demuestra un fallo del cartucho.
- ComprobaciÃ³n: tests host correctos, ROM recompilada, cargada en melonDS Android mediante ADB y captura visual verificada.
- Los indicadores numÃ©ricos del framebuffer requieren glifos explÃ­citos para todos los dÃ­gitos usados. Si faltan `7`, `8` o `9`, una frecuencia puede parecer incorrecta aunque el cÃ¡lculo sea correcto.

- Si algo se atasca y finalmente se resuelve, añadir en el siguiente trabajo una entrada breve en este `AGENTS.md` con: síntoma, causa real, solución y comprobación que lo confirmó.
- Mantener estas reglas actualizadas en vez de repetir el mismo diagnóstico en chats posteriores.

## Verificación mínima antes de entregar

- Ejecutar los tests host de `tests/test_loop_logic.cpp` y `tests/test_loop_logic.py`.
- Compilar la ROM con `build.ps1`.
- Si ADB está disponible, desplegar y arrancar la ROM en el dispositivo Android antes de dar por terminada la tarea.
- No afirmar que el audio físico está validado solo por compilar: distinguir siempre entre validación de lógica, emulador y hardware.

# AGENTS.md — DS Stylophone / Looper

Este archivo contiene únicamente reglas vigentes y contexto estable para trabajar en el proyecto. No es un diario de sesiones ni un registro de bugs históricos.

## Fuente de verdad y mantenimiento

- `STATUS.md` resume el estado actual del producto.
- `KNOWN_ISSUES.md` contiene únicamente limitaciones o bugs abiertos que todavía afectan al trabajo.
- `.codex-journal/` conserva el historial de conversaciones; sirve como contexto histórico, no como instrucciones actuales.
- Al resolver un bug, actualizar `STATUS.md` y eliminarlo de `KNOWN_ISSUES.md` en el mismo cambio. No añadir a este archivo el relato del síntoma, hipótesis, intentos o comandos usados para resolverlo.
- Los bloqueos puntuales ya resueltos pertenecen al historial o a la documentación del script que los evita; solo una regla general reutilizable debe quedar aquí.
- Mantener este archivo pequeño: cada regla debe prevenir un error recurrente o describir una decisión técnica que siga siendo válida. Revisarlo al cerrar una tarea y retirar reglas que ya no apliquen.
- Mantener actualizado `MANUAL.md` en el mismo cambio que modifique controles, interfaz, timbres, grabación, reproducción, diario o cualquier otra funcionalidad visible del juego.

## Flujo de depuración y validación

- BlocksDS es el único toolchain y build canónico del proyecto. `.tools/blocksds-test` es la copia aislada de compilación, pero la única ROM final es `ds-stylophone.nds` en la raíz.
- Usar `.tools/nogba-debug/NO$GBA.EXE` (Debugger Fullversion) para trazas y depuración de CPU/memoria. Pasar la ROM entre comillas porque la ruta contiene espacios.
- NO$GBA es human-in-the-loop: para validar TTY abrir `Window > TTY Debug Messages`, pulsar `Copy` y dejar que el script compruebe el portapapeles. No asumir que existe `DEBUGMSG.TXT` ni automatización fiable de la GUI.
- Mantener melonDS como emulador oficial de compatibilidad, regresión visual y audio. NO$GBA complementa esa validación; no la sustituye.
- No usar Android, ADB ni DeSmuME para el ciclo normal del proyecto.
- Cerrar NO$GBA antes de recompilar para evitar bloqueos de `.map` y otros artefactos.
- En NO$GBA activar `Options > Debugger Setup > Debug I/O at 4FFFAxx > Enabled (TTY Messages etc.)`. Para ROMs BlocksDS, desactivar los avisos modales en `Options > Debug eXception Setup > Disable all Warnings`; son falsos positivos conocidos de I/O y DTCM (`0x02FF4000..0x02FF7FFF`), no una prueba de corrección.
- Las pruebas NO$GBA no validan audio. No introducir notas durante ellas; silenciar el emulador si se necesita garantizar silencio.

## Comandos de trabajo

### Lógica, build y flujo del looper

```powershell
python tests/test_loop_logic.py
g++ -std=c++17 -Iinclude tests/test_loop_logic.cpp -o tests/test_loop_logic.exe
.\tests\test_loop_logic.exe
.\build.ps1
.\tools\run-looper-flow.ps1
```

El resultado esperado del flujo es `LOOPER_FLOW_RESULT=PASS`. Para una prueba auditiva explícita usar `-EnableSound`.

### Smoke BlocksDS + NO$GBA

```powershell
.\tools\validate-nogba-smoke.ps1 -ManualTty
```

Debe terminar con `NOGBA_SMOKE_RESULT=PASS` y contener `SMOKE|STDERR|boot` y `SMOKE|NOCASH|boot` copiados desde esa ejecución.

### ROM real BlocksDS + NO$GBA

```powershell
.\tools\validate-nogba-blocksds-real.ps1
```

Recompila la copia BlocksDS y exige comprobar visualmente la interfaz y copiar una TTY que contenga `DSDBG|BOOT|blocksds|frame` y `DSDBG|NOCASH|boot`. El resultado esperado es `NOGBA_BLOCKSDS_REAL_RESULT=PASS`.

### Distribución de la ROM

```powershell
.\tools\upload-rom-ftp.ps1
```

Este script recompila la ROM canónica con `.\build.ps1` y sube exactamente `ds-stylophone.nds` a `/roms/nds/`. No usar ni mantener un segundo build devkitPro: PC, emulador, FTP y Nintendo DS deben recibir siempre esa misma ROM.

## Invariantes técnicas

- Las 12 zonas táctiles son cromáticas, de anchura idéntica y usan exactamente los índices `0`–`11`; no mostrar nombres de notas.
- El índice táctil y el dibujo usan `key = (x * 12) / 256`, con `x` calibrada en `0..255`.
- Probar todos los tonos, especialmente `E4=329 Hz`, `F4=349 Hz` y el límite `B4 < C5`.
- Dibujar en framebuffer de RAM y copiar a VRAM; no redibujar la bitmap completa directamente en VRAM cada frame.
- Las etiquetas de las 12 notas deben existir en el framebuffer.
- Las notas discretas usan `soundPlayPSG`; reservar `soundSetFreq` para glide y vibrato.
- El PSG recibe `8 * Hz`; conservar los Hz musicales para la interfaz y las comprobaciones.
- Al diagnosticar audio, mostrar temporalmente nota, MIDI y Hz para separar mapeo, reproducción y mezcla con loops.

## Criterio de entrega

- Separar siempre validación de lógica, validación del emulador y validación de audio físico.
- No declarar validado el audio físico ni la consola DSi sin evidencia explícita de esa prueba.
- Si aparece un bloqueo nuevo y se resuelve, documentar solo la regla reutilizable o la corrección en el script correspondiente; no añadir una cronología a este archivo.

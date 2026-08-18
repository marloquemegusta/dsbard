# Estado

- El sistema de grabacion ya no depende de una pista master. Cualquier pista puede grabarse primero.
- Cada pista contiene frases seriales. Una captura se abre con `A`, conserva el timing desde ese instante, pasa por recorte START/END y se anade al final de la pista al aceptar.
- Cada frase puede repetirse `xN`; las copias se expanden en serie. Las tres pistas comparten un timeline global y la cancion completa vuelve a empezar al terminar la pista mas larga.
- La franja inferior de 32 px de la pantalla tactil se usa temporalmente para el recorte. `START` reproduce o detiene la preview; `A` acepta; `B` descarta; arriba/abajo modifica las repeticiones.
- Funciona: interfaz directa, 12 zonas tactiles numeradas, octavas, tres pistas, frases seriales, sustain, mute, pausa, undo de la ultima frase, vibrato, glide y diario literal de 256 tokens.
- Cada `NoteEvent` conserva `instrument_id`; hay seis instrumentos PCM multisampleados y cuatro referencias PSG (`SQUARE`, `PULSE`, `THIN`, `DARK`).
- El audio conserva las frecuencias musicales en Hz y usa `8 * Hz` para PSG. Los tests cubren monotonia `E -> F` en octavas 2-6.
- `build.ps1` sincroniza `source/main.cpp` e `include/loop_logic.h` con `.tools/blocksds-test`, compila BlocksDS y copia la salida a la unica ROM final `ds-stylophone.nds`.
- Probado: tests host-side C++ y estructurales; build DS reproducible con BlocksDS.
- Validaciones pendientes: ver `KNOWN_ISSUES.md`; aun no se puede afirmar la verificacion auditiva formal ni la validacion fisica en DSi XL.
- Build exacto: `.\build.ps1` (BlocksDS canonico).
- ROM final: `ds-stylophone.nds`.

# Estado

- El sistema de grabacion ya no depende de una pista master. Cualquier pista puede grabarse primero.
- Cada pista contiene frases seriales. Una captura se abre con `A`, conserva el timing desde ese instante, pasa por recorte START/END y se anade al final de la pista al aceptar.
- Cada frase puede repetirse `xN`; las copias se expanden en serie. Las tres pistas comparten un timeline global y el transporte puede repetir la cancion completa o una ventana de frases.
- El monitor superior representa la estructura por frases: cada repeticion se dibuja como una caja independiente, no como actividad nota a nota.
- La aceptación usa la superficie táctil completa para seleccionar bloques de notas, editar START/END o moverlos en cuartos de pulso; las capturas nuevas redondean automáticamente inicio y duración al cuarto de `PULSE`; `START` reproduce o detiene la preview; `A` acepta; `B` descarta.
- Durante la preview de recorte, una barra verde recorre el gráfico inferior para indicar la posición dentro del rango elegido.
- El transporte admite pausa, loop opcional y una selección temporal global para las tres pistas. `Y` solo conmuta el loop; sin loop se detiene al final y `START` reinicia la selección.
- `Y` decide el loop al soltarse; no cambia la selección temporal ni introduce un selector de frases.
- La franja tactil `160..192` muestra sliders `PULSE 250..2000 ms`, `VIB 0..10` y `SUS 1..10` en modo normal. `PULSE` solo es reloj interno para rejilla y snap; el metrónomo audible está desactivado. `FOCUS TIMELINE`, activado manteniendo `SELECT`, lleva el editor de notas de la pista seleccionada a la pantalla táctil y deja el teclado arriba; no muestra playhead ni tiradores de rango. `VIB 0` desactiva el vibrato; `L` mantiene el sustain y `SUS 10` alcanza una cola de 2.88 s.
- Funciona: interfaz directa, 12 zonas tactiles numeradas, octavas, tres pistas, frases seriales, sustain, mute, pausa, loop por selección temporal, undo de la ultima frase, vibrato y diario literal de 256 tokens sin separadores.
- La grabacion separa el fin de la pulsacion de la cola de sustain; cada evento recuerda si `L` estaba activo y el nivel `SUS` usado. En aceptación, `L` corto alterna el sustain de la nota seleccionada y `S` lo marca visualmente.
- Cada `NoteEvent` conserva `instrument_id` y su nivel de sustain; hay seis instrumentos PCM multisampleados y cuatro referencias PSG (`SQUARE`, `PULSE`, `THIN`, `DARK`).
- El audio conserva las frecuencias musicales en Hz y usa `8 * Hz` para PSG. Los tests cubren monotonia `E -> F` en octavas 2-6.
- `build.ps1` sincroniza `source/main.cpp` e `include/loop_logic.h` con `.tools/blocksds-test`, compila BlocksDS y copia la salida a la unica ROM final `ds-stylophone.nds`.
- Probado: tests host-side C++ y estructurales; build DS reproducible con BlocksDS.
- Validaciones pendientes: ver `KNOWN_ISSUES.md`; aun no se puede afirmar la verificacion auditiva formal ni la validacion fisica en DSi XL.
- Build exacto: `.\build.ps1` (BlocksDS canonico).
- ROM final: `ds-stylophone.nds`.

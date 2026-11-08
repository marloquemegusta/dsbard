# Estado

- El sistema de grabacion ya no depende de una pista master. Cualquier pista puede grabarse o componerse primero.
- Al entrar en una cancion (`NEW` o `CONTINUE`), la aplicacion abre directamente el **Compositor de 2 octavas** (`focus_timeline = true`, `DEFAULT_PULSE_MS = 500`):
  - **Pantalla superior (`COMPOSER`)**: muestra la vista macro (`0S..20S`) de las 4 pistas en bandas continuas (`y = 30..153`), un recuadro verde de 1 px sobre la pista activa indicando la ventana visible de 5 s (`0S..5S`), un sub-recuadro azul claro indicando la ventana `DETAIL 2P` cuando esta abierta, y las barras inferiores separadas `DIARY` y `PULSE 500 S  LOOP  VIB 0  SUS 2`.
  - **Pantalla inferior (Piano Roll + Teclado de 2 filas)**: muestra la cabecera compacta (`TRK N > STATE  OCT 4 5  TIMBRE`), la regla interactiva de 5 s (`0S..5S`, tocar/arrastrar mueve el playhead), la escalera vertical de octavas a la izquierda (`7..2` con indicador verde deslizante para hacer pan vertical de 24 semitonos), el piano roll de la pista activa y la franja inferior (`y = 143..191`).
  - **Teclado cromatico de 2 filas (`0..11`) y botones de octava (`5 +` / `4 -`)**: la fila superior toca `octave + 1` y la fila inferior toca `octave`, diferenciando teclas naturales y sostenidos; la columna izquierda permite subir/bajar octava con un toque.
  - **Edicion inline `DETAIL 2P`**: arrastrar una tecla `0..11` al piano roll inserta la nota directamente en la pista activa y sustituye solo la franja del teclado inferior por la barra `DETAIL 2P` (zoom de 2 pulsos) para mover o redimensionar la nota sin salir del compositor. Tocar una nota existente tambien abre `DETAIL 2P`; pulsar `A` o tocar un hueco vacio del piano roll devuelve el teclado de 2 filas; `B` borra la nota seleccionada.
  - **Ayuda en vivo (`SELECT` mantenido)**: mantener `SELECT` muestra el overlay `CONTROLS` de 15 lineas en la pantalla superior sin salir del compositor; un toque corto de `SELECT` silencia o activa la pista seleccionada.
- La tipografia bitmap 3x5 incluye glifos completos (`A-Z`, `0-9`, `|`, `-`, `+`, `/`, `:`, `.`, `>`, `#`) y `DRAW A NAME` interpola trazos continuos al arrastrar el stylus.
- Recuperado de las sesiones: almacenamiento binario v1 compatible con 16 temas (`DSSG`), biblioteca de arranque (`CONTINUE`/`NEW`/`SONGS`), etiquetas manuscritas y guardado atomico tras cambios de notas, transporte, mute y efectos. Si la tarjeta no esta disponible, la aplicacion continua en memoria.
- Cada `NoteEvent` conserva `instrument_id` y su nivel de sustain; hay **13 instrumentos PCM multisampleados** (`RHODES`, `KALIMBA`, `BASS`, `PLUCK`, `GLASS`, `PAD`, `HEROIC`, `HORN`, `ANIME`, `CINEMA`, `STRINGS`, `E GUITAR`, `FLUTE`, 52 archivos `.bin`) con sus puntos de loop exactos y **4 referencias PSG** (`SQUARE`, `PULSE`, `THIN`, `DARK`), sumando 17 timbres.
- El audio conserva las frecuencias musicales en Hz y usa `8 * Hz` para PSG. Los tests cubren monotonia `E -> F` en octavas 2-6.
- `build.ps1` sincroniza las fuentes, cabeceras y las 52 muestras con `.tools/blocksds-test`, compila BlocksDS y copia la salida a la unica ROM final `ds-stylophone.nds`.
- Probado: tests host-side C++ y estructurales; compilacion DS canonica con BlocksDS en Docker, incluyendo `source/song_files.cpp`.
- Comparacion y regresion: la ROM reconstruida valida contra la ROM de referencia de la DS (`recovered/ds-stylophone-from-ds.nds`) en los escenarios headless de DeSmuME (`tests/scenarios/composer_headless.json` y los escenarios de auditoria visual/funcional).
- Validaciones pendientes: ver `KNOWN_ISSUES.md`; aun no se puede afirmar la verificacion auditiva formal ni la validacion fisica en DSi XL.
- Build exacto: `.\build.ps1` (BlocksDS canonico).
- ROM final: `ds-stylophone.nds`.

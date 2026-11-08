# Manual de DS Stylophone / Looper

## 1. Que es el juego

DS Stylophone / Looper es un cuaderno musical e instrumento cromático para Nintendo DS y DSi. Al abrir o crear una canción entras directamente en el **Compositor de 2 octavas**: la pantalla inferior ofrece un teclado táctil de dos filas (`0`-`11`) junto al piano roll de la pista activa, mientras la pantalla superior muestra el mapa general de las cuatro pistas (`0S..20S`), el diario de notas recientes y el estado de pulso y efectos.

## 2. Pantalla y conceptos principales

- **Teclado táctil de 2 filas (`0`-`11`)**: situado en la franja inferior (`y = 143..191`). La fila superior interpreta la octava superior (`octave + 1`) y la fila inferior la octava base (`octave`), diferenciando visualmente teclas naturales y sostenidos, siempre con números relativos `0`-`11`.
- **Botones táctiles de octava (`5 +` / `4 -`)**: en la esquina inferior izquierda (`x = 0..17`), tocar la mitad superior (`+`) sube la octava base y tocar la mitad inferior (`-`) la baja (rango `2..6`).
- **Piano Roll (pantalla inferior, `y = 28..139`)**: muestra una ventana temporal de 5 segundos (`0S..5S`) y una ventana vertical de 24 semitonos (2 octavas) para la pista seleccionada (`TRK 1..4`).
- **Escalera vertical de octavas (`7..2`)**: en el margen izquierdo del piano roll (`x = 0..18`), un bloque verde indica qué región de octavas está visible; arrastrar verticalmente por esa columna desplaza la ventana de alturas (`PITCH_PAN`).
- **Regla interactiva (`0S..5S`)**: tocar o arrastrar sobre la regla (`y = 16..27`) reposiciona inmediatamente la aguja verde de reproducción (`SEEK_RULER`).
- **Vista macro superior (`COMPOSER`, `0S..20S`)**: muestra las cuatro pistas apiladas de un vistazo. La pista seleccionada resalta con un recuadro verde de 1 px la ventana activa de 5 s (`0S..5S`), y con un sub-recuadro azul claro la ventana de edición `DETAIL 2P` cuando está abierta.
- **Edición inline `DETAIL 2P`**: al arrastrar una nota desde el teclado `0`-`11` y soltarla en el piano roll (o al tocar una nota ya existente), la franja del teclado inferior se transforma en la barra **`DETAIL 2P`** (zoom de 2 pulsos) sin tapar el piano roll. Arrastrar el centro mueve la nota; arrastrar sus extremos izquierdo o derecho cambia su inicio o duración; pulsar `A` o tocar un espacio vacío del piano roll cierra `DETAIL 2P` y devuelve el teclado de 2 filas; pulsar `B` elimina la nota.
- **Ayuda rápida (`SELECT` mantenido)**: mantener pulsado `SELECT` muestra en la pantalla superior la hoja de referencia `CONTROLS` mientras se mantiene pulsado; soltar `SELECT` vuelve a mostrar el timeline macro. Un toque corto de `SELECT` silencia o activa (`MUTE`/`PLAY`) la pista actual.
- **Diario (`DIARY`)**: registro temporal de las últimas notas tocadas en vivo (`0`-`11`).

## 3. Controles

| Control | Acción |
|---|---|
| Teclado táctil (`0`-`11`, 2 filas) | Tocar para preescuchar en vivo; arrastrar hacia arriba al piano roll para insertar una nota en la pista activa y abrir `DETAIL 2P`. |
| Botones táctiles `+` / `-` (`x < 18`) | Subir o bajar la octava base (`2..6`). |
| Escalera izquierda (`7..2`, `x <= 24`) | Arrastrar verticalmente para desplazar la ventana de 2 octavas del piano roll. |
| Regla temporal (`0S..5S`, `y = 16..27`) | Tocar o arrastrar para mover el playhead a cualquier instante de la ventana. |
| Nota en el Piano Roll | Tocar para seleccionarla y abrir `DETAIL 2P`; arrastrar para moverla; tocar hueco vacío para cerrar `DETAIL 2P` y volver al teclado. |
| Barra `DETAIL 2P` (`y = 143..191`) | Arrastrar el centro de la barra verde para mover la nota o sus bordes para ajustar inicio/fin dentro de los 2 pulsos. |
| D-pad arriba/abajo | Cambiar de pista (`TRK 1..4`); si hay una nota seleccionada, subir o bajar su altura en semitonos. |
| D-pad izquierda/derecha | Cambiar de pista (`TRK 1..4`); si hay una nota seleccionada, desplazarla en cuartos de pulso. |
| `L` + D-pad arriba/abajo | Ampliar o reducir el zoom temporal del viewport del compositor. |
| `A` | Si `DETAIL 2P` está abierto, aceptar/cerrar el detalle y volver al teclado; si no, iniciar o cerrar grabación en vivo. |
| `B` | Si hay una nota seleccionada, borrarla (`DELETE NOTE`). |
| `L` + `B` | Deshacer (`UNDO`) la última edición o frase. |
| `R` + `B` | Limpiar (`CLEAR`) por completo la pista seleccionada. |
| `X` | Cambiar al siguiente timbre (17 instrumentos disponibles). |
| `Y` | Activar o desactivar `LOOP` al soltarlo. |
| `L` mantenido al tocar | Activar cola de sustain (`SUS`). |
| `R` | Activar o silenciar el clic del metrónomo global (`PULSE`). |
| `START` | Pausar o reanudar la reproducción (`PLAY/PAUSE`). |
| `SELECT` corto | Silenciar o activar (`MUTE`) la pista seleccionada. |
| `SELECT` mantenido | Mostrar el panel de ayuda `CONTROLS` en la pantalla superior. |

## 4. Timbres (17 instrumentos)

Los 17 timbres disponibles (13 instrumentos PCM multisampleados + 4 ondas PSG) son:

1. `RHODES` (PCM)
2. `KALIMBA` (PCM)
3. `BASS` (PCM)
4. `PLUCK` (PCM)
5. `GLASS` (PCM)
6. `PAD` (PCM con loop)
7. `HEROIC` (PCM con loop)
8. `HORN` (PCM con loop)
9. `ANIME` (PCM con loop)
10. `CINEMA` (PCM con loop)
11. `STRINGS` (PCM con loop)
12. `E GUITAR` (PCM con loop)
13. `FLUTE` (PCM con loop)
14. `SQUARE` (PSG)
15. `PULSE` (PSG)
16. `THIN` (PSG)
17. `DARK` (PSG)

Cada evento guarda su propio `instrument_id`, por lo que distintas notas y pistas pueden combinar timbres libremente.

## 5. Biblioteca y guardado

Al arrancar aparecen `CONTINUE` y `NEW`:
- **`CONTINUE`**: abre la biblioteca `SONGS` con las canciones guardadas (`00..15`). Toca una canción o pulsa `A` para cargarla, `B` corto para volver atrás o mantén `B` para borrarla.
- **`NEW`**: abre la pantalla `DRAW A NAME`. Arrastra el stylus dentro del recuadro para dibujar el nombre manuscrito con trazos continuos, pulsa `X` para limpiar el dibujo, `A` para aceptar y entrar al compositor o `B` para volver.
- Todos los cambios en notas, pistas y parámetros se guardan automáticamente de forma atómica (`DSSG`).

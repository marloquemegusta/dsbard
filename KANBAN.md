# Kanban de features — DS Stylophone / Looper

Este archivo es la copia versionada del tablero. La interfaz visual es [`KANBAN.html`](KANBAN.html). Para cambiar muchas tarjetas cómodamente, abre el HTML, arrástralas y pulsa **Exportar Markdown**; sustituye este archivo por el descargado y revisa el diff.

## Reglas rápidas

- `Inbox`: ideas sin aclarar. No se implementan todavía.
- `Todo`: trabajo entendido, priorizado y listo para empezar.
- `En curso`: máximo 2 tarjetas. Solo mover aquí la tarjeta que se está implementando.
- `Revisión`: implementación terminada, pendiente de pruebas, revisión o documentación.
- `Hecho`: criterio de terminado satisfecho y documentación actualizada cuando corresponda.
- Una tarjeta debe describir una entrega pequeña, tener criterio de terminado y conservar su ID.
- **K-007 — Sustain: separar duración grabada y cola de reproducción** [bug, audio, grabación] — **Revisión**
  - Registrar el fin real de la pulsación aunque `L` mantenga una cola, y reproducir esa cola sin convertirla en una nota mantenida durante toda la duración.
  - Terminado: pruebas host y build correctos; una captura con `L` produce un evento con fin al soltar la franja, conserva el timbre y reproduce una cola limitada por `SUS`; actualizar estado y manual.
- **K-008 — Y: decidir loop al soltar tras comprobar navegación** [bug, ux, looper] — **Hecho**
  - Posponer la conmutación de loop hasta soltar `Y`; izquierda/derecha (y el ajuste de frases) consume la pulsación sin apagar el loop.
  - Terminado: prueba de lógica para soltar `Y` sin acción y para navegar mientras se mantiene; build correcto y comportamiento documentado.
- **K-009 — Playhead verde en la preview de recorte** [feature, ux, grabación] — **Revisión**
  - Mostrar una barra verde que avance sobre el gráfico inferior mientras `START` reproduce el rango recortado.
  - Terminado: la barra sigue el tiempo de preview dentro de `START..END`, desaparece al detener y la documentación queda actualizada.
- **Revisión**
- **K-005 — Panel FX compacto para vibrato y sustain** [ux, audio]
  - Quitar glide y reutilizar la franja `160..192`: controles FX en modo normal y timeline de recorte durante la aceptación.
  - Terminado: vibrato y cola de sustain ajustables con sliders tactiles cuantizados, sustain activado manteniendo `L`, sin tocar las 12 zonas cromaticas; tests, build y documentación actualizados.
## Revisión

- **K-004 — Corregir píxeles sobrantes en los tiradores de recorte** [bug, ux, grabación]
  - Separar visualmente los tiradores verdes `START`/`END` de sus etiquetas para que las letras no invadan la barra.
  - Terminado: tiradores rectangulares sin píxeles salientes en la captura del menú de recorte; pruebas de lógica y build correctas.


## Inbox

- **K-002 — Guardar sesiones entre ejecuciones** [producto, looper]
  - Definir si el instrumento debe persistir canciones, frases o diario al salir.
  - Terminado: Decisión documentada; si se implementa, pruebas de compatibilidad y recuperación.

- **K-003 — Revisar ideas de UX del instrumento** [ux]
  - Recoger mejoras de controles, ayuda y feedback sin romper la interfaz de 12 zonas.
  - Terminado: Lista priorizada con alcance y criterio de aceptación por idea.

## Revisión

- **K-010 — Timeline táctil, pulso y corrección rítmica mínima** [feature, ux, looper, ritmo, grabación] — **Revisión**
  - **Fricción observada:** al repetir muchas veces una base no se puede volver inmediatamente al comienzo útil; `LOOP 1` repite las tres copias de una captura `x3` porque la UI muestra tres cajas pero el transporte cuenta una `Phrase` lógica; y el intérprete comete errores grandes de duración o posición que un autoajuste no puede interpretar.
  - **Objetivo:** sustituir `LOOP N` por una selección temporal directa y reutilizar una sola rejilla de pulso para transporte, referencias visuales y corrección manual. `PULSE` es solo un reloj interno: el metrónomo audible queda desactivado. Tocar normalmente no debe exigir tempo, compás, proyecto ni editor.
  - **Conceptos públicos permitidos:**
    1. `FOCUS TIMELINE`: una acción de un solo botón intercambia las pantallas y lleva el monitor de pistas a la pantalla táctil. Estado visible y reversible, no editor genérico.
    2. `PULSE GRID`: duración del pulso y subdivisión fija de cuatro pasos por pulso. No añadir todavía compás, acentos ternarios/cuaternarios, nombres de notas ni cuantización automática.
  - **Modelo mínimo:**
    - `TimelineSelection { cursor_ms, range_start_ms, range_end_ms, explicit_range }`.
    - `loop_enabled` es independiente. Con loop, al alcanzar `range_end_ms` vuelve a `range_start_ms`; sin loop, se detiene. Sin selección explícita, el rango es `0..song_duration`.
    - El rango afecta al timeline global y mantiene sincronizadas las tres pistas, aunque se elija sobre la pista seleccionada.
    - `PulseGrid` convierte sin deriva mediante `time(step) = step * pulse_ms / 4`; no acumular repetidamente `pulse_ms / 4`.
  - **Pack A, obligatorio primero — timeline táctil y transporte:**
    - Elegir una acción de un solo botón libre o recolocar explícitamente una función secundaria; no introducir otro acorde. Documentarla en `MANUAL.md`.
    - En `FOCUS TIMELINE`: toque corto coloca `cursor_ms` y `transport_ms`; arrastre horizontal crea un rango ordenado `INICIO..FIN`; después se pueden arrastrar ambos tiradores. Mostrar selección, playhead y pista de referencia sin ocultar las tres pistas.
    - Snap híbrido: sin pulso, al borde de caja de repetición más cercano; con rejilla, a cuartos de pulso, dando prioridad a un borde de caja cercano. Una captura `x3` expone tres cajas seleccionables aunque internamente siga siendo una `Phrase` con `repeat_count=3`.
    - `Y` solo conmuta `LOOP ON/OFF`. Eliminar la interacción y texto público `LOOP N`/`loop_phrase_count`; no hace falta cambiar el almacenamiento de `Phrase` ni desduplicar eventos.
    - `START` reproduce/pausa. Si terminó, reinicia en `range_start_ms`. Buscar el cursor no apaga el loop ni altera el rango.
    - Extraer a `include/loop_logic.h` lógica host-testable para normalizar/clamp del rango, fin con/sin loop, bordes de repeticiones expandidas y snap.
  - **Pack B — pulso compartido:**
    - Añadir pulso configurable antes de grabar con una interacción pequeña y visible; mostrar primero el concepto cotidiano, por ejemplo `PULSO 1.00 s`. Mantener un valor por defecto y permitir tocar/grabar sin configurarlo.
    - Durante grabación, `PULSE` solo mantiene la rejilla y el reloj interno; no emitir clic de metrónomo ni guardarlo como `NoteEvent`.
    - Dibujar pulsos y cuartos de pulso en el timeline enfocado. Clic y marcas comparten `PulseGrid` y origen temporal.
    - No añadir compás: ternario/cuaternario solo será necesario para acentos o agrupaciones.
  - **Pack C — corrección automática y manual de la captura recién grabada:**
    - Ampliar la aceptación para usar la pantalla táctil; no meter edición nota a nota en los 32 px actuales.
    - Cada `NoteEvent` es un bloque seleccionable. Ofrecer targets cómodos para moverlo, cambiar `START` y cambiar `END`, en pasos de `1/4` de pulso.
    - Al cerrar la captura, redondear automáticamente cada inicio y duración al cuarto de pulso más cercano, con duración mínima de un cuarto; después, los bordes se pueden corregir manualmente y los eventos deben seguir sin solaparse.
    - Los silencios son huecos y no hay ripple. Impedir duración cero, salida del rango y solapamiento con notas vecinas. No modificar pitch, `instrument_id` ni `sustain_on_release`.
    - `START` alterna preview; `A` acepta; `B` descarta recuperando exactamente la captura original. Conservar repeticiones o recolocarlas en controles táctiles documentados.
    - Reutilizar `build_trim_preview`, `accept_trim` y `PulseGrid`; no crear otro formato ni motor de playback.
  - **Pack D, solo tras validar A-C — frases anteriores:**
    - Desde `FOCUS TIMELINE`, tocar una caja abre el mismo editor sobre una frase aceptada. No crear un segundo editor.
    - Para `repeat_count > 1`, la opción mínima es editar la fuente y reflejar el cambio en todas las repeticiones. Editar una copia aislada requiere otra decisión de producto y queda fuera.
  - **Fuera de alcance:** secuenciador general, piano roll por alturas, mover pitch, overdub, persistencia, zoom complejo, compás, cuantización automática global, edición simultánea de varias frases o rediseño de las 12 zonas.
  - **Orden para el agente:** leer `AGENTS.md`, `STATUS.md`, `KNOWN_ISSUES.md`, `MANUAL.md`, `include/loop_logic.h` y las zonas relevantes de `source/main.cpp`; mover `K-010` a `En curso` antes de editar; implementar y verificar A, luego B y luego C. No iniciar D sin A-C completas. Ante una decisión perceptible no fijada, preguntar.
  - **Terminado / evidencia para Revisión:**
    1. Tests host nuevos cubren rango normal/invertido, clamp, seek, reinicio, overshoot en loop, parada sin loop, bordes expandidos de `x3` y conversión step-tiempo sin deriva; los existentes siguen pasando.
    2. Regresión obligatoria: `Phrase` de 1 s con `repeat_count=3` dibuja tres cajas; seleccionar la tercera da `2 s..3 s`; loop repite solo ese segundo; sin loop para en 3 s; `START` vuelve a 2 s.
    3. Tres pistas con duraciones distintas obedecen el mismo rango sin reinicios independientes.
    4. Una nota demasiado larga se acorta varios cuartos y otra se desplaza; no hay solapamiento; preview/aceptación conserva pitch, timbre y sustain; cancelar conserva tiempos originales.
    5. Añadir trazas temporales bajo `DS_DEBUG_TRACE` para `FOCUS`, `SEEK`, `RANGE`, `WRAP/STOP`, `GRID` y `NOTE_EDIT`, con tiempos y resultado; no mostrar diagnóstico técnico en UI final.
    6. Ejecutar `python tests/test_loop_logic.py`, compilar/ejecutar `tests/test_loop_logic.cpp`, `build.ps1` y `tools/run-looper-flow.ps1` hasta `LOOPER_FLOW_RESULT=PASS`.
    7. Crear escenario melonDS específico: intercambiar pantallas, seleccionar una caja de `x3`, revisar visualmente `INICIO/FIN`/playhead y probar loop/parada. Inspeccionar capturas; un hash no basta. Para audio, prueba auditiva intencional con `-EnableSound`; no afirmar validación física DSi.
    8. Actualizar `MANUAL.md`, `STATUS.md` y, solo si queda un problema real, `KNOWN_ISSUES.md`. Mover a `Revisión`, no a `Hecho`, hasta validar en hardware.
  - **Guía de diagnóstico:**
    - Si se repite `x3` completo, revisar `Phrase.start_ms + repetition * duration_ms`; no usar `phrase_count` como número de cajas.
    - Si el playhead salta, trazar `transport_ms`, límites y overshoot; aplicar módulo sobre el span conservando overshoot.
    - Si la rejilla deriva, comprobar origen y `time(step)` comunes; no mantener dos acumuladores ni reactivar un clic audible.
    - Si editar una nota altera otra, comparar copia original/editable y clamps contra `previous.end_ms`/`next.start_ms`.
    - Validar helpers host-side antes de emulador y recoger solo trazas o escenarios pertinentes según `AGENTS.md`.

## Todo

- **K-001 — Validación auditiva formal y DSi XL** [audio, hardware]
  - Comprobar audio con una prueba repetible y validar la ROM en hardware físico.
  - Terminado: Evidencia explícita de ambas pruebas; actualizar STATUS.md y KNOWN_ISSUES.md.

## Revisión

## Hecho

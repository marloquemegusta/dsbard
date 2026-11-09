# Kanban de features — DS Stylophone / Looper

Este archivo es la copia versionada del tablero. La interfaz visual es [`KANBAN.html`](KANBAN.html). Para cambiar muchas tarjetas cómodamente, abre el HTML, arrástralas y pulsa **Exportar Markdown**; sustituye este archivo por el descargado y revisa el diff.

## Reglas rápidas

- `Inbox`: ideas sin aclarar. No se implementan todavía.
- `Todo`: trabajo entendido, priorizado y listo para empezar.
- `En curso`: máximo 2 tarjetas. Solo mover aquí la tarjeta que se está implementando.
- `Revisión`: implementación terminada, pendiente de pruebas, revisión o documentación.
- `Hecho`: criterio de terminado satisfecho y documentación actualizada cuando corresponda.
- Una tarjeta debe describir una entrega pequeña, tener criterio de terminado y conservar su ID.

## Inbox

- **K-002 — Guardar sesiones entre ejecuciones** [producto, looper]
  - Definir si el instrumento debe persistir canciones, frases o diario al salir.
  - Terminado: Decisión documentada; si se implementa, pruebas de compatibilidad y recuperación.

- **K-003 — Revisar ideas de UX del instrumento** [ux]
  - Recoger mejoras de controles, ayuda y feedback sin romper la interfaz de 12 zonas.
  - Terminado: Lista priorizada con alcance y criterio de aceptación por idea.

## Todo

- **K-010 — Timeline táctil, metrónomo y corrección rítmica mínima** [feature, ux, looper, ritmo, grabación]
- **K-001 — Validación auditiva formal y DSi XL** [audio, hardware]
  - Comprobar audio con una prueba repetible y validar la ROM en hardware físico.
  - Terminado: Evidencia explícita de ambas pruebas; actualizar STATUS.md y KNOWN_ISSUES.md.

## En curso

## Revisión

- **K-005 — Panel FX compacto para vibrato y sustain** [ux, audio]
  - Quitar glide y reutilizar la franja `160..192`: controles FX en modo normal y timeline de recorte durante la aceptación.
  - Terminado: vibrato y cola de sustain ajustables con sliders tactiles cuantizados, sustain activado manteniendo `L`, sin tocar las 12 zonas cromaticas; tests, build y documentación actualizados.

- **K-004 — Corregir píxeles sobrantes en los tiradores de recorte** [bug, ux, grabación]
  - Separar visualmente los tiradores verdes `START`/`END` de sus etiquetas para que las letras no invadan la barra.
  - Terminado: tiradores rectangulares sin píxeles salientes en la captura del menú de recorte; pruebas de lógica y build correctas.

- **K-007 — Sustain: separar duración grabada y cola de reproducción** [bug, audio, grabación]
  - Registrar el fin real de la pulsación aunque `L` mantenga una cola, y reproducir esa cola sin convertirla en una nota mantenida durante toda la duración.
  - Terminado: pruebas host y build correctos; una captura con `L` produce un evento con fin al soltar la franja, conserva el timbre y reproduce una cola limitada por `SUS`; actualizar estado y manual.

- **K-009 — Playhead verde en la preview de recorte** [feature, ux, grabación]
  - Mostrar una barra verde que avance sobre el gráfico inferior mientras `START` reproduce el rango recortado.
  - Terminado: la barra sigue el tiempo de preview dentro de `START..END`, desaparece al detener y la documentación queda actualizada.

- **K-012 — Rediseño: compositor por descubrimiento de notas y paridad con ROM de la DS** [feature, ux, looper, composición]
  - Historia de usuario: al escuchar una canción, quiero probar notas en el teclado de 2 octavas y colocarlas directamente en una línea temporal por pista, con ajuste fino inline (`DETAIL 2P`), vista macro de 20s arriba y los 17 timbres de la ROM de referencia.
  - Terminado: tests host en verde, build BlocksDS sin errores y capturas comparativas de los escenarios de auditoría alineadas en estilo y funcionalidad con la ROM de referencia.

## Hecho

- **K-016 — Control de Tempo (BPM) global con escalado temporal de eventos** [feature, ux, audio, composición] — **Hecho**
  - **Alcance implementado:**
    1. Atajo con gatillo `R`: `R + ←` ralentiza el tempo (-5 BPM / aumenta `pulse_ms`), `R + →` acelera (+5 BPM / reduce `pulse_ms`) en rango 60..240 BPM.
    2. Escalado temporal proporcional de notas y duraciones de todas las pistas para preservar el ritmo, la duración musical y las relaciones intactas.
    3. Visualización clara: indicador `BPM` en la cabecera del compositor táctil y en la barra de estado superior.
    4. Soporte táctil dual: tocar la mitad izquierda de `BPM` en la cabecera ralentiza (-5 BPM) y la derecha acelera (+5 BPM) con el stylus.
    5. Pulsación de `R` pospuesta al soltar: evita conmutar accidentalmente el metrónomo al usar combinaciones con `R` (`R + ←`/`→` o `R + B`).
    6. Ayuda `CONTROLS` y documentación en `MANUAL.md` y `STATUS.md` actualizadas.
  - **Validación:** tests de lógica en host C++ (`test_loop_logic.cpp`) y Python (`test_loop_logic.py`) en verde; flujo integrado `run-looper-flow.ps1` con `LOOPER_FLOW_RESULT=PASS`; compilación limpia BlocksDS en `ds-stylophone.nds`.

- **K-008 — Y: decidir loop al soltar tras comprobar navegación** [bug, ux, looper] — **Hecho**
  - Posponer la conmutación de loop hasta soltar `Y`; izquierda/derecha (y el ajuste de frases) consume la pulsación sin apagar el loop.
  - Terminado: prueba de lógica para soltar `Y` sin acción y para navegar mientras se mantiene; build correcto y comportamiento documentado.

- **K-013 — Fase 1: Limpieza de vistas, Compases 4/4 y 3/4, Ghost Notes y Controles del Compositor** [feature, ux, compositor] — **Hecho**
  - **Alcance implementado:**
    1. Eliminado el modo `DETAIL 2P` y `TRIM`: el teclado cromático de 2 filas (`0..11`, `y=143..191`) con botones de octava `+`/`-` permanece siempre visible y operativo.
    2. Rectángulo amarillo/naranja en la pantalla macro superior (`0S..20S`) que delimita exactamente la ventana visible en la pantalla inferior.
    3. Soporte para alternar compás entre `4/4` y `3/4` tocando la cabecera, con fondo en cebra alterna por compás y líneas de pulso claras.
    4. Sombras de pistas (Ghost Notes): las demás pistas activas se muestran en gris suave con su índice `0..11` para componer armonías.
    5. Controles completos del compositor: preescucha + estampar con `A`, edición de tono con cruceta `↑`/`↓`, desplazamiento temporal con `←`/`→`, redimensión de duración con `L + ←`/`→`, selección rectangular con `L + Stylus`, clonado de bloques/compases con `L + A`, borrado con `B`, pan libre con stylus en vacío y reproducción contextual desde el inicio del compás con `START`.
  - **Validación:** tests host en C++ (`test_loop_logic.cpp`) y Python (`test_loop_logic.py`) en verde; compilación limpia BlocksDS en `ds-stylophone.nds`.

- **K-014 — Fase 2: Rediseño sonoro, volumen/envolventes y paleta de 10 instrumentos + kit percusión** [audio, timbres] — **Hecho**
  - **Alcance implementado:**
    1. Motor de volumen y compensación corregido: volumen nominal unificado a `95/127` para preescucha y loop, con compensación suave de graves/agudos.
    2. Envolvente por software ADSR (ataque suave de 15 ms, decay y sostenido al 70%) para las voces PSG (`CHIP LEAD`, `CHIP WAVE`), eliminando clics y pitidos planos.
    3. Paleta definitiva de 10 timbres: `BRASS` (heroico), `STRINGS` (cálidas), `CHOIR` (coro gregoriano con formantes *"Ohhh"* en graves y *"Ahhh"* en agudos), `FLUTE` (con aire), `PIANO` (Rhodes de caída natural), `PLUCK` (arpa), `BASS` (redondo para altavoces DS), `CHIP LEAD` (pulso 50% con envolvente), `CHIP WAVE` (pulso 12% con envolvente) y `DRUMS` (kit de percusión de 12 sonidos asignados a las teclas `0..11`).
  - **Validación:** muestras binarias integradas, tests C++ y Python pasando, `run-looper-flow.ps1` finalizado con `LOOPER_FLOW_RESULT=PASS` y compilación canónica en `build.ps1`.

- **K-015 — Fase 3: Documentación, pantalla de ayuda CONTROLS y Manual del Pequeño Compositor** [docs, ux] — **Hecho**
  - **Alcance implementado:**
    1. Pantalla de ayuda `CONTROLS` (`SELECT` mantenido) actualizada con los controles reales del compositor.
    2. `MANUAL.md` actualizado con los nuevos diagramas, controles, los 10 timbres y la sección "Manual del Pequeño Compositor" con guía de compases `4/4` vs `3/4`, sombras de notas y reglas armónicas numéricas (`+0`, `+3/+4`, `+7`).
    3. `STATUS.md` y `SAMPLES.md` actualizados eliminando menciones a sistemas obsoletos.
  - **Validación:** documentación completa, sin contradicciones y alineada con la especificación de `DESIGN.md`.

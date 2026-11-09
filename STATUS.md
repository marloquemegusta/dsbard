# Estado

- Al entrar en una canción (`NEW` o `CONTINUE`), la aplicación abre directamente el **Compositor cromático de 2 octavas** (`focus_timeline = true`, `DEFAULT_PULSE_MS = 500`):
  - **Pantalla superior (`COMPOSER`)**: vista macro (`0S..20S`) de las 4 pistas, un recuadro amarillo/naranja que encuadra la ventana activa que se visualiza en la pantalla inferior, aguja de reproducción verde global y tiras inferiores `DIARY` y de estado (`PULSE`, `LOOP`, `VIB`, `SUS`).
  - **Pantalla inferior (Piano Roll + Teclado cromático permanente)**:
    - Teclado cromático de 2 filas (`0..11`, `y = 143..191`) con botones `+`/`-` de octava a la izquierda: **nunca desaparece ni se oculta** durante la selección, edición o inserción.
    - Cabecera interactiva con conmutador de métrica `4/4` vs `3/4` e indicador táctil de tempo (`BPM`).
    - Control de Tempo / BPM (`60..240 BPM`): `R + ←`/`→` ajusta la velocidad en pasos de ±5 BPM (o tocando la mitad izquierda/derecha del indicador `BPM` en la cabecera); todas las notas, compases y duraciones se escalan proporcionalmente en tiempo real para preservar el ritmo de la canción intacto.
    - Metrónomo en `R`: pulsación simple conmuta clic de metrónomo al soltar, sin dispararse accidentalmente al usar `R + ←`/`→` o `R + B`.
    - Fondo de piano roll en cebra por compás con líneas de pulso claras.
    - Sombras de pistas (Ghost Notes): las demás pistas no silenciadas se proyectan como siluetas grises con su índice `0..11`.
    - Escalera vertical de octavas a la izquierda (`7..2`) para pan de alturas.
    - Regla interactiva con búsqueda inmediata de aguja.
  - **Flujo de inserción y edición ágil**:
    - Preescucha en vivo al tocar cualquier tecla `0..11`; botón `A` estampa la última nota en el cursor respetando la rejilla de pulso.
    - Arrastrar desde el teclado al piano roll inserta directamente donde se suelte.
    - Edición de notas seleccionadas: cruceta `↑`/`↓` transpone tono con preescucha; `←`/`→` desplaza en el tiempo; `L + ←`/`→` redimensiona duración por el extremo derecho; `B` borra selección.
    - Edición directa con stylus: arrastrar centro mueve la nota; arrastrar borde derecho cambia su duración.
    - Selección rectangular múltiple manteniendo `L` y arrastrando el stylus sobre el piano roll.
    - Clonado instantáneo con `L + A` para duplicar la selección o el último compás a continuación.
    - Desplazamiento libre (pan) arrastrando el stylus sobre cualquier zona vacía del piano roll.
    - Reproducción con contexto en `START`: reproduce desde el inicio del compás actual.
  - **Ayuda en vivo (`SELECT` mantenido)**: muestra el panel `CONTROLS` de 15 líneas en la pantalla superior; un toque corto conmuta silencio (`MUTE`) en la pista activa.
- **Paleta de sonido y motor de volumen nivelado**:
  - Eliminada la compensación agresiva antigua de `pcm_volume_for_note`; ecualización suave centrada en C4 y volumen normalizado a `95/127` tanto para preescucha como para reproducción en bucle.
  - Envolvente ADSR por software (15 ms de ataque suave, decay y sustain al 70%) para las voces PSG (`CHIP LEAD`, `CHIP WAVE`), eliminando pitidos planos y clics.
  - **10 instrumentos definitivos**:
    1. `BRASS` (heroico multisampleado con loop)
    2. `STRINGS` (cuerdas cálidas con loop)
    3. `CHOIR` (coro gregoriano con formantes *"Ohhh"* en graves y *"Ahhh"* en agudos)
    4. `FLUTE` (flauta con aire y loop suave)
    5. `PIANO` (Rhodes/piano de caída natural)
    6. `PLUCK` (arpa/cuerda pulsada)
    7. `BASS` (bajo definido y redondo para altavoces DS)
    8. `CHIP LEAD` (onda pulso 50% con envolvente)
    9. `CHIP WAVE` (onda pulso 12% con envolvente)
    10. `DRUMS` (kit de percusión de 12 elementos mapeado a las teclas `0..11`)
- Almacenamiento binario v1 compatible con 16 temas (`DSSG`), biblioteca de arranque (`CONTINUE`/`NEW`/`SONGS`), etiquetas manuscritas y guardado atómico tras cambios de notas, transporte, mute y efectos.
- `build.ps1` sincroniza fuentes, cabeceras y muestras con `.tools/blocksds-test`, compila con BlocksDS y genera la única ROM canónica `ds-stylophone.nds`.
- Validaciones completadas: tests estructurales Python (`test_loop_logic.py`), tests de lógica en host C++ (`test_loop_logic.cpp`), flujo integrado de looper (`run-looper-flow.ps1` con `LOOPER_FLOW_RESULT=PASS`) y compilación limpia BlocksDS.
- Build exacto: `.\build.ps1` (BlocksDS canónico).
- ROM final: `ds-stylophone.nds`.

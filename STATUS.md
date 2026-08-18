# Estado
- La pista `1` es master: fija la duración real `T`; las pistas `2` y `3` esperan su primera nota, anclan la toma al ciclo master actual y ajustan su duración a cualquier múltiplo entero de `T`.

- Funciona: interfaz directa, 12 zonas táctiles numeradas, octavas, tres tracks estáticos con duración y origen independientes, grabación/reproducción de eventos, sustain grabado hasta soltar L, mute y pausa con fase, undo/reemplazo, vibrato, glide y diario literal de 256 tokens.
- Añadidos seis instrumentos PCM multisampleados (cuatro raíces por instrumento) y cuatro referencias PSG (`SQUARE`, `PULSE`, `THIN`, `DARK`) con selección circular en X; cada `NoteEvent` conserva `instrument_id` para que los tracks mantengan su timbre original.
- Ajustado el balance PCM/PSG: PCM parte de un nivel más cercano al PSG y aplica compensación progresiva de volumen en notas graves, sin cambiar la frecuencia calculada.
- Controles rediseñados: L es sustain durante el toque, L+izquierda activa vibrato y L+derecha glide; B gestiona el diario; L+B deshace la última regrabación; R+B limpia la pista; Y inserta separadores sin stylus; SELECT corto silencia y mantenido muestra ayuda.
- La representación visible de notas es numérica; MIDI se conserva solo internamente para audio y loops.
- `build.ps1` sincroniza `source/main.cpp` e `include/loop_logic.h` con `.tools/blocksds-test`, compila BlocksDS y copia su salida a la única ROM final `ds-stylophone.nds`; `run-emulator.ps1` lanza esa misma ROM.
- Corregido el mapeo PSG: las frecuencias musicales se convierten a `8 * Hz` en notas discretas, glide y vibrato; los tests cubren monotonía `E -> F` en octavas 2–6.
- Ajustado el muestreo táctil para aceptar cambios de zona en el primer frame y reducido el redibujado de la pantalla superior a 30 Hz; entrada, audio y loops siguen a 60 Hz para mejorar glides rápidos en hardware DS.
- El diario usa exclusivamente valores literales `0..11`, uno por franja pulsada, sin tener en cuenta octava, MIDI ni frecuencia.
- `SELECT` y `B` usan estados explícitos de pulsación larga y liberación para mostrar ayuda y limpiar el diario de forma fiable.
- Probado: tests host-side C++ y estructurales; build DS reproducible con BlocksDS; ROM cargada en melonDS 1.1; captura visual confirma ambas pantallas y el arrastre del stylus sobre una franja.
- Validaciones pendientes: ver `KNOWN_ISSUES.md`; el entorno no permite afirmar todavía la verificación auditiva formal ni la validación física en DSi XL.
- Build exacto: `.\build.ps1` (BlocksDS canónico).
- ROM final: `ds-stylophone.nds`.

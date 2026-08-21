# Rediseño del compositor por descubrimiento de notas

Documento de recuperación para retomar el trabajo en otro chat. Describe la intención del usuario y el resultado que debe perseguirse; no es una descripción de la implementación experimental abandonada.

## Estado

- Estado: propuesta pendiente de rediseño.
- Base para retomar: `master`, desde `v1.4` y la documentación posterior.
- Rama `codex/note-composer-redesign`: experimento abandonado; no continuar ni fusionar.
- El proceso de subagentes/equipo usado en ese experimento queda deprecado para este proyecto.

## Historia de usuario

Como persona que escucha una canción y quiere reconstruirla de oído, quiero usar el teclado táctil para descubrir la siguiente nota y colocarla directamente en una composición temporal, para poder construir una base y añadir capas sin tener que interpretar perfectamente a tempo ni corregir una grabación de frases después.

La experiencia deseada es:

```text
escucho → pruebo notas → encuentro una que encaja → la coloco → ajusto su duración → repito
```

El usuario no quiere manejar un DAW ni aprender una abstracción de frases. Quiere un instrumento pequeño para componer descubriendo una nota cada vez.

## Fricción que debe resolver

Cuando intento reproducir una melodía, tengo que tocarla a tempo para grabarla dentro de una frase. Si fallo la duración o entro tarde, la frase queda mal y debo editarla después. Las frases, sus repeticiones, sus estados de grabación y sus controles añaden complejidad sin ayudarme a descubrir la siguiente nota.

## Flujo objetivo

1. Se inicia una sesión sin proyecto, BPM, compás ni pista base obligatorios.
2. Se selecciona una de las tres pistas.
3. El ciclo global existente puede reproducirse en bucle.
4. El usuario toca el teclado para preescuchar notas. La preescucha nunca modifica la composición.
5. Cuando encuentra la siguiente nota, la añade directamente a la pista en una posición temporal.
6. La nota aparece seleccionada en el compositor.
7. El usuario arrastra la nota para moverla y su borde para cambiar su duración.
8. Reproduce el resultado y repite el proceso.
9. Selecciona otra pista y descubre una capa nueva sobre la base ya existente.
10. Puede corregir o borrar una nota sin abrir un editor de frases ni reiniciar el ciclo.

La colocación debe ser una acción directa y reversible. No introducir una confirmación `A/B` como sustituto permanente de una buena interacción; si hace falta una protección temporal, debe justificarse y medirse contra un gesto directo con deshacer.

## Modelo musical mínimo

La unidad pública y editable es la nota, no la frase.

```text
Composition
  loop_length
  tempo o pulse
  tracks[3]

Track
  notes[]

Note
  start_tick
  duration_ticks
  pitch relativo 0..11
```

Decisiones iniciales:

- tres pistas fijas;
- un reloj y un ciclo global compartido;
- posiciones y duraciones discretas en ticks/pulsos para no capturar la latencia humana;
- sin polifonía, velocity, automatización, overdub ni persistencia en la primera versión;
- el teclado conserva exactamente las 12 zonas cromáticas `0..11`;
- la reproducción usa el motor de audio existente;
- `Phrase` puede conservarse internamente durante una migración, pero no debe ser necesaria para crear o editar notas nuevas.

## Interfaz deseada

La pantalla inferior debe dedicar la mayor parte de su espacio al compositor:

```text
┌──────────────────────────────┐
│ regla temporal                │
│ pista 1  ──■────■───────────  │
│ pista 2  ─────■──────■──────  │
│ pista 3  ─────────■─────────  │
├──────────────────────────────┤
│ teclado cromático compacto    │
│  0  1  2  3  4  5 ... 11      │
└──────────────────────────────┘
```

La pantalla superior comunica transporte, ciclo, pista seleccionada, mute/solo y una vista general. El compositor no debe estar escondido detrás de un selector de frases ni requerir entrar en un modo de aceptación.

Controles mínimos:

- tocar tecla: preescucha;
- tocar hueco temporal: insertar nota;
- arrastrar nota: mover;
- arrastrar borde: cambiar duración;
- borrar/undo: retirar la última nota o la nota seleccionada;
- reproducir/parar;
- cambiar pista;
- mute/solo;
- cambiar octava.

## Escenario de aceptación

La prueba de uso será reconstruir aproximadamente 20 segundos de una referencia musical escuchada externamente, por ejemplo la introducción de Juego de Tronos, sin exigir una transcripción exacta.

El usuario debe poder:

1. construir una base sencilla de cuatro o cinco notas;
2. reproducirla en un ciclo global;
3. descubrir una melodía encima mediante preescucha;
4. colocar cada nota directamente sin tocarla a tempo perfecto;
5. ajustar posiciones y duraciones;
6. añadir una segunda pista sincronizada;
7. equivocarse, borrar o mover una nota y continuar;
8. comparar base y melodía mediante mute/solo.

La prueba debe poder describirse como una secuencia de controles visible y corta. Si el usuario necesita alternar repetidamente entre captura, aceptación, selector, editor y reproducción, el diseño ha vuelto a introducir la complejidad que debía eliminar.

## Reutilización esperada

Antes de crear estructuras nuevas, inspeccionar y reutilizar:

- `NoteEvent`;
- `PulseGrid`;
- `TimelineSelection`;
- reloj monotónico y transporte global;
- motor de voz y `soundPlayPSG`;
- teclado táctil de 12 zonas;
- framebuffer y copiado a VRAM;
- helpers host-testable de selección, snap y edición.

La migración debe ser gradual. No borrar `Phrase` hasta que exista una representación segura de notas directas y una estrategia para conservar o descartar composiciones antiguas.

## Fuera de alcance inicial

- piano roll completo;
- zoom complejo;
- compases y teoría musical visible;
- cuantización global automática;
- edición de pitch después de colocar la nota;
- polifonía;
- velocity y automatización;
- overdub;
- loops independientes por pista;
- persistencia;
- nombres absolutos de notas.

## Criterios de terminado de la primera implementación real

- La pantalla inicial muestra claramente el nuevo compositor o permite llegar a él sin un modo de frases oculto.
- El teclado solo preescucha cuando se usa como descubrimiento.
- Añadir una nota no exige grabar una interpretación completa.
- La nota se puede mover, alargar, borrar y reproducir inmediatamente.
- Las tres pistas comparten un único reloj.
- Un error normal no deja estados parciales ni obliga a reiniciar.
- La lógica de notas directas tiene tests host-side.
- El playback existente se reutiliza o se modifica de forma localizada; no se crea un segundo motor.
- La interfaz, el manual y el estado del producto describen el mismo flujo.
- La validación de lógica se separa de la validación visual en emulador y de la validación de audio en hardware.

## Qué falló en el experimento anterior

- Se implementó una rebanada experimental accesible detrás de `SELECT`, mientras la pantalla inicial seguía expresando el modelo antiguo. Por eso la ROM no comunicaba el rediseño.
- Se mantuvo demasiado flujo de frases: selector, estados provisionales y confirmación `A/B`.
- Se intentó validar una arquitectura grande antes de fijar una superficie visible mínima.
- El proceso multiagente añadió rondas y commits, pero no garantizó que el resultado coincidiera con la intención de producto.
- Una notificación de agente afirmó haber ejecutado melonDS pese a la prohibición; esa evidencia se descartó. No repetir el modo equipo.

La lección principal es que la primera versión debe cambiar de forma visible la unidad de interacción, no solo añadir una ruta experimental compatible con el sistema antiguo.

## Cómo retomar en otro chat

Comenzar desde `master`, leer este documento y formular primero una propuesta de interfaz visible y un flujo de cinco minutos. No continuar la rama `codex/note-composer-redesign` ni sus commits como base de producto.

La primera pregunta de diseño debe ser:

> ¿Cuál es la superficie mínima que permite preescuchar una nota y colocarla directamente en una pista sin entrar en el modelo de frases?

Después de aprobar esa superficie, implementar una única rebanada vertical pequeña, validarla en la ROM y decidir si merece crecer.

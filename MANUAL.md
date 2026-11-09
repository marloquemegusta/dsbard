# Manual de DS Stylophone / Looper

## 1. Qué es el instrumento

DS Stylophone / Looper es un cuaderno musical e instrumento cromático para Nintendo DS y DSi. Al abrir o crear una canción entras directamente en el **Compositor de 2 octavas**: la pantalla inferior ofrece un piano roll de edición ágil y un teclado táctil cromático de dos filas (`0`–`11`) que **nunca desaparece**, mientras la pantalla superior muestra la vista macro general de las cuatro pistas (`0S..20S`), el diario de notas recientes y los parámetros musicales activos.

El instrumento favorece el oído y el pensamiento interválico sobre la teoría formal: todas las notas se expresan exclusivamente en índices relativos `0`–`11`.

---

## 2. Pantalla y conceptos principales

### Pantalla Inferior (Táctil) — Piano Roll y Teclado

- **Teclado cromático de 2 filas (`0`–`11`)**: situado en la franja inferior (`y = 143..191`), permanentemente visible.
  - La fila superior toca la octava alta (`octave + 1`).
  - La fila inferior toca la octava base (`octave`).
  - Ambas filas diferencian visualmente teclas naturales y sostenidos mediante sombreado sutil.
- **Botones táctiles de octava (`+` / `-`)**: en el margen inferior izquierdo (`x = 0..17`), tocar la mitad superior sube la octava base y tocar la inferior la baja (rango `2..6`).
- **Piano Roll (`y = 28..139`)**: muestra el pentagrama cromático de 2 octavas (24 semitonos) para la pista seleccionada (`TRK 1..4`).
  - **Fondo en cebra y líneas de compás**: las columnas alternan un sombreado suave por compás según la métrica elegida (`4/4` o `3/4`), con líneas sólidas para delimitar cada compás y líneas tenues para cada pulso.
  - **Sombras de otras pistas (Ghost Notes)**: las notas de las demás pistas no silenciadas aparecen como siluetas grises translúcidas con su número `0`–`11` de fondo, permitiendo componer acompañamientos y armonías sin tapar la melodía principal.
- **Escalera vertical de alturas (`7..2`)**: en el margen izquierdo (`x = 0..18`), indica qué rango de octavas está visible; arrastrar verticalmente por esta columna desplaza el piano roll en tono (`PITCH_PAN`).
- **Regla interactiva (`y = 16..27`)**: tocar o arrastrar sobre la regla temporal desplaza inmediatamente la aguja verde de reproducción (`SEEK_RULER`).
- **Alternar compás (`4/4` vs `3/4`)**: tocar el indicador de métrica en la cabecera conmuta instantáneamente entre 4 pulsos por compás y 3 pulsos por compás.
- **Ajustar Tempo / BPM (`60..240 BPM`)**: el tempo activo se muestra en la cabecera táctil (ej. `120BPM`) y en la barra de estado superior. Tocar la mitad izquierda del indicador ralentiza la canción (-5 BPM) y la mitad derecha la acelera (+5 BPM). Con el gatillo `R`, pulsar `R + ←` o `R + →` ajusta la velocidad inmediatamente con la cruceta. Todas las notas y compases se escalan proporcionalmente en tiempo real.

### Pantalla Superior — Vista Macro y Estado

- **Vista macro (`COMPOSER`, `0S..20S`)**: muestra el mapa general de las 4 pistas.
  - **Rectángulo amarillo/naranja**: encuadra con nitidez la ventana temporal exacta de la pista activa que se está viendo y editando en la pantalla inferior.
  - **Aguja verde**: marca la posición actual del transporte de audio en toda la canción.
- **Diario (`DIARY`)**: tira horizontal que recuerda en tiempo real las últimas notas tocadas en vivo (`0`–`11`), sirviendo como andamio de memoria musical.
- **Barra de estado**: muestra la pista activa, el timbre seleccionado, el tempo/rejilla, el estado de `LOOP` y efectos.
- **Hoja de ayuda (`SELECT` mantenido)**: mantener pulsado `SELECT` despliega al instante el panel de referencia `CONTROLS` en la pantalla superior; al soltarlo, regresa la vista macro.

---

## 3. Controles

### Tocar e Insertar Notas
| Control | Acción |
|---|---|
| **Teclado táctil (`0`–`11`)** | Tocar una tecla la reproduce en vivo con volumen balanceado y la recuerda como última nota tocada. |
| **Botón `A` (Estampar)** | Estampa la última nota tocada en el cursor temporal/a continuación de la nota anterior (respetando la rejilla de pulso) y la selecciona. |
| **Arrastrar desde el teclado** | Arrastrar el stylus desde cualquier tecla `0`–`11` hacia el piano roll inserta la nota exactamente en el tiempo y semitono donde se suelte. |
| **Botones táctiles `+` / `-`** | Subir o bajar la octava base de interpretación (`2..6`). |

### Edición de Nota(s) Seleccionada(s)
| Control | Acción |
|---|---|
| **Cruceta `↑` / `↓`** | Sube o baja la altura de la selección en semitonos (`±1`, con preescucha breve). |
| **Cruceta `←` / `→`** | Desplaza la nota o el bloque seleccionado en el tiempo (`±1` paso de rejilla). |
| **`L` + Cruceta `←` / `→`** | Alarga o acorta la duración de la selección por su extremo derecho (`±1` paso). |
| **Stylus en el centro de una nota** | Arrastrar desplaza la nota en tiempo y altura. |
| **Stylus en el borde derecho** | Arrastrar ajusta la duración de la nota interactivamente. |
| **Botón `B`** | Elimina la nota o todas las notas del bloque seleccionado (`DELETE`). |
| **`L` + Stylus (Selección Rectangular)** | Mantener `L` y arrastrar sobre el piano roll dibuja un marco de selección para seleccionar múltiples notas a la vez. |
| **`L` + `A` (Duplicar / Clonar)** | Clona las notas seleccionadas (o el compás actual) y las estampa justo a continuación, moviendo la selección a la copia. |

### Navegación y Estructura (Sin selección activa)
| Control | Acción |
|---|---|
| **Stylus en zona vacía del piano roll** | Arrastrar hace scroll/pan libre y continuo tanto en el tiempo horizontal como en el tono vertical. |
| **Cruceta `↑` / `↓`** | Cambia de pista activa (`TRK 1` a `TRK 4`). |
| **Cruceta `←` / `→`** | Salta la vista y la aguja de reproducción 1 compás hacia atrás o hacia adelante. |
| **`L` + Cruceta `←` / `→`** | Zoom temporal Out / In en el piano roll. |
| **`R` + Cruceta `←` / `→`** | Ralentizar / Acelerar el tempo global (`±5 BPM`, rango `60..240 BPM`). Escala proporcionalmente todas las notas y compases en tiempo real. |
| **Tocar cabecera `4/4` / `3/4`** | Alterna la métrica del compás entre `4/4` y `3/4`. |
| **Tocar cabecera `BPM`** | Tocar la mitad izquierda (-5 BPM) o derecha (+5 BPM) para ajustar el tempo con el stylus. |

### Transporte y Sistema
| Control | Acción |
|---|---|
| **`START`** | Reproducir o pausar. Si se edita una zona, inicia la reproducción desde el principio del compás actual para escuchar el contexto musical. |
| **`X`** | Cambiar al siguiente timbre de la paleta (10 instrumentos disponibles). |
| **`Y`** | Conmutar repetición de ciclo (`LOOP ON` / `LOOP OFF`) al soltar. |
| **`SELECT` (toque corto)** | Silenciar o reactivar la pista seleccionada (`MUTE`). |
| **`SELECT` (mantener)** | Mostrar la hoja de ayuda de controles en la pantalla superior. |
| **`L` + `B`** | Deshacer (`UNDO`) la última acción de edición. |
| **`R` + `B`** | Limpiar (`CLEAR`) todas las notas de la pista seleccionada. |
| **`R` (pulsación simple)** | Activar o silenciar el clic del metrónomo al soltar (no se activa accidentalmente al usar `R + ←`/`→` o `R + B`). |

---

## 4. Paleta de Sonido (10 Instrumentos)

El motor de audio ha sido nivelado para equilibrar la sonoridad percibida (RMS) entre muestras PCM y generadores PSG en todas las octavas (`2..6`), con un volumen uniforme de preescucha y reproducción (`95/127`).

1. **`BRASS`**: metal orquestal heroico y redondo, brillante sin chirriar en agudos (loop continuo).
2. **`STRINGS`**: sección de cuerdas cálidas con ataque suave y sostenido envolvente (loop continuo).
3. **`CHOIR`**: coro épico gregoriano multisampleado. Responde con formantes vocálicos profundos tipo *"Ohhh"* en registros graves (`octavas 2..3`) y vocales luminosas tipo *"Ahhh"* en registros medios y agudos (`octavas 4..6`).
4. **`FLUTE`**: flauta dulce con soplido de aire en el ataque y cuerpo estable (loop continuo).
5. **`PIANO`**: sonido percusivo y dulce estilo Rhodes/acústico con caída armónica natural (one-shot).
6. **`PLUCK`**: cuerda pulsada / arpa limpia de caída rápida (one-shot).
7. **`BASS`**: bajo acústico/eléctrico con fundamental limpio y armónicos controlados para los altavoces de la Nintendo DS (one-shot).
8. **`CHIP LEAD`**: generador PSG retro de onda pulso cuadrada al 50% con envolvente ADSR por software (ataque suave de 15 ms, decay y sostenido al 70%) para evitar clics y estridencias.
9. **`CHIP WAVE`**: generador PSG retro de pulso al 12% con envolvente ADSR, ideal para melodías solistas y arpegios chiptune.
10. **`DRUMS`**: kit completo de 12 elementos de percusión acústica y electrónica distribuidos directamente en las 12 teclas del teclado `0`–`11`:
    - **`0`**: Bombo profundo (*Kick*)
    - **`1`**: Timbal orquestal grave (*Low Tom*)
    - **`2`**: Timbal orquestal medio (*Mid Tom*)
    - **`3`**: Timbal orquestal agudo (*High Tom*)
    - **`4`**: Caja limpia y seca (*Snare Clean*)
    - **`5`**: Caja épica con reverb amplia (*Epic Snare*)
    - **`6`**: Rimshot acústico / Palma (*Clap*)
    - **`7`**: Tom electrónico retro
    - **`8`**: Plato charles cerrado (*Closed Hi-hat*)
    - **`9`**: Plato charles abierto (*Open Hi-hat*)
    - **`10`**: Plato de choque (*Crash Cymbal*)
    - **`11`**: Cortinilla / Campana orquestal (*Chime*)

---

## 5. Biblioteca y Guardado Automático

Al encender el instrumento dispones de dos opciones:
- **`CONTINUE`**: abre la biblioteca `SONGS` con hasta 16 canciones guardadas (`00..15`). Toca una canción o pulsa `A` para abrirla, pulsa `B` para volver o mantén `B` para borrarla.
- **`NEW`**: abre el lienzo `DRAW A NAME` para dibujar a mano alzada el título de tu canción. Pulsa `A` para aceptar y entrar al compositor.
- **Guardado Atómico**: cada nota insertada, movida o borrada, así como los cambios de compás, silencios y timbres, se guardan instantáneamente en la tarjeta SD de forma transparente sin interrumpir la música.

---

## 6. Manual del Pequeño Compositor

Esta sección explica cómo hacer música agradable en la DS de forma intuitiva, sin necesidad de saber solfeo ni armonía tradicional.

### 6.1. El Compás: ¿Marcha (`4/4`) o Vals (`3/4`)?

La cabecera del compositor te permite alternar entre `4/4` y `3/4` con solo tocarla:
- **`4/4` (4 pulsos por compás — Cebra de 4 tiempos)**:
  - Es el ritmo natural del pop, rock, marcha y música electrónica (*UN, dos, tres, cuatro*).
  - El pulso fuerte cae siempre en el primer tiempo (la línea divisoria gruesa de cada columna de cebra).
  - Pon el bombo (`DRUMS 0`) en el pulso 1 y 3, y la caja (`DRUMS 4` o `5`) en el 2 y 4 para una base sólida instantánea.
- **`3/4` (3 pulsos por compás — Cebra de 3 tiempos)**:
  - Es el compás del vals, las canciones de cuna y las baladas melancólicas (*UN, dos, tres / UN, dos, tres*).
  - Coloca una nota grave de bajo en el pulso 1 y dos acordes o notas agudas en los pulsos 2 y 3 (*¡pum, chas, chas!*).

### 6.2. Usar las Sombras de Pistas (Ghost Notes) para no tropezar

Cuando estás en la pista 2 (por ejemplo haciendo un bajo o arpegio) y tienes una melodía grabada en la pista 1, verás sus notas dibujadas como siluetas grises translúcidas con sus números `0`–`11`:
- **Evita choques rítmicos**: si ves una sombra en un pulso, puedes decidir si quieres que tu nota caiga exactamente en el mismo momento para acentuarla, o en los huecos intermedios para responderle (pregunta y respuesta).
- **Armonía visual**: verás a qué altura está la melodía sin tener que recordarla de memoria.

### 6.3. La Regla Numérica de los Acordes (`0`–`11`)

En este instrumento no hay nombres de notas (`C`, `D`, `F#`), solo distancias entre teclas (`0` a `11`). Esta relación nunca cambia:

- **La Tónica (`+0`)**: la nota principal que eliges como raíz.
- **La Tercera (El color emocional: ¿Alegría o Melancolía?)**:
  - **Tercera Menor (`+3` semitonos)**: suena nostálgica, triste, misteriosa o épica.
  - **Tercera Mayor (`+4` semitonos)**: suena luminosa, heroica, alegre y abierta.
- **La Quinta Justa (`+7` semitonos)**: el soporte de estabilidad. Encaja siempre con cualquier raíz y le da cuerpo sin definir si es triste o alegre (*Power Chord*).

#### Fórmulas de Construcción Rápida:
| Tipo de Acorde | Fórmula Interválica | Ejemplo empezando en 0 | Ejemplo empezando en 5 |
|---|---|---|---|
| **Acorde Mayor** (Alegre / Heroico) | `Raíz` + `4` + `7` | `0` – `4` – `7` | `5` – `9` – `0` *(12 vuelve a 0)* |
| **Acorde Menor** (Épico / Melancólico) | `Raíz` + `3` + `7` | `0` – `3` – `7` | `5` – `8` – `0` |
| **Quinta Vacía** (Firme / Medieval) | `Raíz` + `7` | `0` – `7` | `2` – `9` |

> **Consejo práctico al componer:**
> 1. Toca una tecla en la fila inferior para encontrar una nota que te guste (ej. `0`).
> 2. En la pista 1 graba esa nota con `PIANO` o `BRASS`.
> 3. En la pista 2 (`BASS`), pon un `0` en la octava baja.
> 4. En la pista 3 (`STRINGS`), pon un acorde tocando `0`, `3` y `7` (o `0`, `4` y `7`).
> 5. Selecciona el bloque con `L` + Stylus y pulsa `L + A`: ¡tendrás un compás perfectamente duplicado listo para variar con la cruceta `↑`/`↓`!

# DESIGN.md — DS Bard (Compositor por Descubrimiento y Oído en DSi)

> **Propósito de este documento:** Especificación viva de producto, sonido e interacción acordada tras revisar el uso real de la máquina. Sustituye la complejidad acumulada de modos antiguos (grabación en vivo `LIVE REC`, pantalla de recorte `TRIM` y tercer nivel de zoom `DETAIL 2P`) por un flujo único, directo y jugable enfocado en **sacar melodías de oído y montar bandas sonoras / temas chiptune paso a paso**.

---

## 1. Visión: ¿A qué estamos jugando?

Esta máquina no es un DAW profesional ni un looper de reflejos en tiempo real. Es un **juego de construcción musical paso a paso** para quien tiene una melodía en la cabeza (una banda sonora épica, un tema de película o de videojuego) y quiere:

1. **Buscar las notas de oído** en un teclado cromático numérico (`0..11`) sin necesitar solfeo.
2. **Colocarlas en orden sin estrés de tempo**, viendo claramente dónde caen los compases (`4/4` o `3/4`).
3. **Añadir capas épicas (Bajo, Cuerdas, Coros, Metales, Percusión)** ayudándose visualmente de la sombra de las otras pistas y de reglas numéricas sencillas (`+0`, `+3/+4`, `+7`).
4. **Escuchar un resultado que suene redondo, equilibrado y con carácter** en los altavoces de la Nintendo DSi.

**Regla de oro de diseño:** Minimizar el número de clics y modos. Cada acción frecuente (probar nota, meterla a continuación, alargarla, moverla, repetir un compás de batería o borrar un trozo) debe sentirse tan inmediata y predecible como los controles de un videojuego clásico de Nintendo.

---

## 2. Arquitectura Visual: Dos Pantallas, Dos Niveles de Zoom (Cero Modos Ocultos)

Se elimina el tercer nivel de zoom (`DETAIL 2P`) que hacía desaparecer el teclado inferior al tocar una nota, y se elimina el modo de recorte de grabación en vivo (`TRIM`).

Siempre existen exactamente **dos vistas sincronizadas**:

### 2.1. Pantalla Superior — Vista Macro (`0S..20S`)
- Muestra las **4 pistas apiladas** a lo largo de un horizonte amplio (mínimo 20 segundos) para ver la estructura completa de la canción de un vistazo.
- **Rectángulo Amarillo / Naranja de Cámara**: señala exactamente qué porción temporal y qué pista se están viendo con zoom en la pantalla inferior.
- **Barra de estado limpia**: muestra el compás actual (`4/4` o `3/4`), velocidad del pulso (`PULSE`), estado de `LOOP`, e información de la pista/timbre activo.
- **Ayuda rápida (`SELECT` mantenido)**: mientras se mantiene pulsado `SELECT`, muestra la chuleta de controles sin cambiar de modo; al soltarlo, vuelve al mapa macro.

### 2.2. Pantalla Inferior — El Tablero de Juego (Piano Roll + Teclado Fijo)
Dividida en tres zonas permanentes que **nunca cambian de sitio**:

1. **Cabecera y Regla de Compases (`y = 0..27`)**:
   - Selector rápido de Pista (`TRK 1..4`), Instrumento, y Compás (**`4/4` Normal** vs **`3/4` Vals/Épico**).
   - Regla temporal donde tocar o arrastrar mueve la aguja de reproducción (`Playhead`).
2. **Pentagrama / Piano Roll con Zoom (`y = 28..141`)**:
   - Muestra una ventana cómoda de tiempo (ej. 2 a 4 compases) y 2 octavas de altura (con su barra izquierda `7..2` para subir/bajar el visor de octavas).
   - **Fondo en Cebra por Compases**: las columnas de cada compás alternan sutilmente el tono de fondo (claro / oscuro) y marcan las líneas de pulso (`1-2-3-4` o `1-2-3`). Así es imposible perder el pulso "1" o crear notas de duración incoherente sin darse cuenta.
   - **Sombra de Otras Pistas (Ghost Notes)**: las notas de las demás pistas activas se dibujan en gris/transparente de fondo con su número (`0..11`) flojito. Así, al componer el Bajo o los Coros en la Pista 2, ves exactamente dónde caen y qué número tocan las notas de la Melodía en la Pista 1.
3. **Teclado Cromático de 2 Filas (`0..11`) SIEMPRE VISIBLE (`y = 143..191`)**:
   - **Nunca desaparece** al seleccionar o insertar una nota.
   - Fila inferior = octava base (`octave`); fila superior = octava siguiente (`octave + 1`).
   - Botones `+` / `-` a la izquierda para cambiar las octavas del teclado.

---

## 3. Controles: El Flujo de "Videojuego" (Lápiz + Botones Digitales)

Todo se puede hacer de forma natural combinando el **lápiz táctil** (para explorar, colocar o arrastrar) y los **botones físicos** (para acciones exactas y digitales de 1 paso, donde no quieres depender del pulso de la mano).

### 3.1. Descubrir y Meter Notas en Cadena
- **Probar de oído (Preescucha)**: Tocar cualquier tecla `0..11` del teclado inferior hace sonar la nota inmediatamente sin modificar la canción. La máquina recuerda cuál ha sido la **última tecla tocada**.
- **Dos formas de meter la nota en el pentagrama**:
  1. **Con el botón `A` (Estampar en cadena)**: Al pulsar `A` (sin selección múltiple), se inserta la última tecla que acabas de tocar **justo a continuación de la nota anterior** (o en el cursor actual), con imán al pulso de la rejilla, y la deja seleccionada. Si quieres meter otra igual detrás, vuelves a pulsar `A`.
  2. **Con el lápiz (Arrastrar y soltar)**: Arrastras desde la tecla `0..11` hacia arriba hasta el punto del pentagrama donde la quieras colocar.

### 3.2. Editar una Nota Seleccionada (Tono, Posición y Duración)
Cuando una nota está seleccionada (marcada con borde brillante en el pentagrama):
- **Cruceta `↑` / `↓`**: Sube o baja el tono exactamente **1 semitono** (y suena brevemente para que oigas el cambio).
- **Cruceta `←` / `→`**: Mueve la nota hacia la izquierda o derecha en el tiempo (**1 subdivisión de pulso**).
- **`L` mantenido + Cruceta `→`**: **Alarga la nota** (+1 paso por la derecha).
- **`L` mantenido + Cruceta `←`**: **Acorta la nota** (-1 paso por la derecha).
- **Con el lápiz (directo en el pentagrama)**:
  - Arrastrar el cuerpo de la nota la mueve en tiempo y altura.
  - Arrastrar el extremo derecho de la nota la estira o encoge en duración.
- **Botón `B`**: Borra la nota seleccionada.

### 3.3. Selección en Bloque (`L` + Lápiz) y Repetir Compases (`L` + `A`)
Para no tener que copiar a mano una percusión o un bajo compás tras compás, ni sufrir si a mitad de canción quieres cambiar el ritmo:

1. **Seleccionar varias notas a la vez (`L` + Arrastrar en el pentagrama)**:
   - Manteniendo pulsado **`L`**, arrastrar el lápiz sobre el pentagrama dibuja un **recuadro de selección** (como en el escritorio de un PC) que selecciona todas las notas que toque.
   - Con varias notas seleccionadas:
     - **Cruceta `↑` / `↓`**: Sube o baja de tono **todo el bloque a la vez**.
     - **Cruceta `←` / `→`**: Desplaza **todo el bloque** en el tiempo.
     - **Botón `B`**: Borra **todo el bloque** de golpe.
2. **Clonar / Estampar Bloque a continuación (`L` + `A`)**:
   - Si tienes seleccionada una nota o un bloque de notas (por ejemplo, el compás 1 de tu batería o de tu bajo) y pulsas **`L` + `A`**, la máquina **fabrica una copia de esas notas como notas normales justo en el siguiente compás** (respetando la longitud del compás `4/4` o `3/4`) y mueve la selección a la copia recién creada.
   - Así, pulsando **`L` + `A`, `A`, `A`**, rellenas 3 o 4 compases en dos segundos. Y como son notas normales (no un modo especial infinito), cuando llegues al compás 5 puedes parar, borrar un golpe o dibujar un redoble distinto con total libertad.

### 3.4. Navegación cuando NO hay ninguna nota seleccionada
Tocando un hueco vacío del pentagrama se deselecciona la nota actual. En ese estado:
- **Arrastrar con el lápiz en un hueco vacío**: Hace **scroll libre (panning)** horizontal y vertical por el pentagrama.
- **Cruceta `↑` / `↓`**: **Cambia de pista activa (`TRK 1..4`)**.
- **Cruceta `←` / `→`**: **Salta la cámara / aguja 1 compás hacia atrás o hacia adelante**, ideal para avanzar limpiamente al siguiente compás.

### 3.5. Reproducción Contextual (Comprobar si suena bien)
- **`START`**: Reproduce / Pausa.
- **Preescucha con contexto**: Al darle a Play mientras estás editando una zona (o al tocar la regla sobre el compás actual), la aguja arranca desde el inicio de ese compás (o un poquito antes de tu última nota) para que escuches inmediatamente **el salto entre la nota anterior y la nueva**.

### 3.6. Tabla Resumen de Controles

| Control | Con Nota(s) Seleccionada(s) | Sin Selección (Navegación) |
|---|---|---|
| **Teclado `0..11`** | Preescucha nota (y la guarda como última nota tocada) | Preescucha nota (y la guarda como última nota tocada) |
| **Arrastrar Tecla al Pentagrama** | Inserta nota donde sueltes y la selecciona | Inserta nota donde sueltes y la selecciona |
| **Botón `A`** | Estampa otra nota igual justo a continuación | Estampa la última tecla tocada a continuación |
| **`L` + Botón `A`** | **Clona / Duplica** la nota o el bloque seleccionado en el siguiente hueco/compás | Clona el último compás de la pista en el siguiente |
| **Botón `B`** | Borra la nota o el bloque seleccionado | — (`L + B` = Deshacer / Undo último cambio) |
| **Cruceta `↑` / `↓`** | Sube / baja el tono (`±1` semitono) | Cambia de pista (`TRK 1..4`) |
| **Cruceta `←` / `→`** | Mueve la nota/bloque en el tiempo (`±1` paso) | Avanza o retrocede la vista 1 compás |
| **`L` + Cruceta `←` / `→`** | **Acorta / Alarga** la duración de la nota (`±1` paso) | Hace zoom temporal Out / In de la ventana |
| **`L` + Arrastrar Lápiz** | Dibuja un **recuadro de selección múltiple** | Dibuja un **recuadro de selección múltiple** |
| **Arrastrar Lápiz en vacío** | Deselecciona (un toque) o mueve cámara | Mueve la cámara (scroll por el pentagrama) |
| **Botón `X`** | Cambia al siguiente instrumento | Cambia al siguiente instrumento |
| **Botón `Y`** | Activa / desactiva `LOOP` | Activa / desactiva `LOOP` |
| **Botón `START`** | Play / Pause (desde contexto actual) | Play / Pause |
| **Botón `SELECT` (corto)** | Mute / Unmute de la pista actual | Mute / Unmute de la pista actual |
| **Botón `SELECT` (mantenido)** | Muestra chuleta de ayuda `CONTROLS` arriba | Muestra chuleta de ayuda `CONTROLS` arriba |

---

## 4. Diseño Sonoro: Por qué hoy suena mal y la Nueva Paleta de 10 Instrumentos

### 4.1. Diagnóstico técnico de los problemas actuales de sonido
Al inspeccionar `source/main.cpp` y `SAMPLES.md`, los problemas que notas ("suena aleatorio, solo suena bien en algunas octavas, el volumen está súper descuadrado, las ondas suenan demasiado simples") tienen cuatro causas exactas en el código:

1. **Curva de volumen rota por octava (`pcm_volume_for_note`)**:
   - Hoy el código hace `compensation = midi < 72 ? (72 - midi) : 0` y se lo suma al volumen base (`82` en loop, `100` en vivo). Eso significa que en cuanto bajas de la octava 5, el volumen se dispara linealmente hasta topar con `127`, haciendo que los graves retumben o saturen y los agudos se queden flojísimos, además de sonar a distinto volumen cuando tocas tú (`100`) que cuando suena en la pista (`82`).
2. **Descuadre entre ondas PSG y muestras PCM**:
   - Las ondas puras (`SQUARE`, `PULSE`, etc.) se lanzan al hardware PSG con el mismo número de volumen que un sample PCM de 8 bits, cuando acústicamente una onda cuadrada pura tiene mucha más energía media (RMS) que una muestra con caída.
3. **Ondas PSG "muertas" (sin envolvente)**:
   - Ahora mismo las ondas cuadradas se encienden a volumen fijo de principio a fin (`ON/OFF` plano). Un chiptune bueno (tipo Game Boy / NES / Shovel Knight) suena musical porque la nota tiene un **ataque vivo y una caída suave de volumen (envelope)** o un ligero barrido de timbre, no un pitido fijo de microondas.
4. **Samples con puntos de bucle bruscos y estiramiento excesivo**:
   - Varios instrumentos comparten un punto de loop idéntico (`14332`) o se estiran demasiados semitonos lejos de su nota raíz (`48, 60, 72, 84`), deformando el timbre y el vibrato natural.

### 4.2. La Nueva Paleta de 10 Instrumentos (Banda Sonora + Chiptune + Percusión)
En lugar de 17 timbres desiguales, tendremos **10 instrumentos cuidados al milímetro**, normalizados en sonoridad percibida (LUFS/RMS por octava) para que cualquiera de ellos encaje con los demás sin tocar volúmenes:

#### Bloque 1: Banda Sonora Épica (Orquestal / Cine)
1. **`BRASS` (Metales Heroicos)**:
   - Sonido cálido y redondo de trompas/trompetas (estilo *Star Wars / Zelda*), con ataque firme pero sin estrépito metálico en octavas altas.
2. **`STRINGS` (Cuerdas de Cine)**:
   - Entrada suave (arco) y cuerpo mantenido muy estable para hacer camas armónicas o melodías largas emotivas.
3. **`CHOIR` (Coro Épico / Gregoriano)**:
   - Voces humanas sintetizadas con formantes claros: en las octavas graves (`2..3`) resuena como un **coro masculino gregoriano (*"Ohhh"* profundo)**, y en las octavas medias/agudas (`4..6`) se abre hacia un **coro épico luminoso (*"Ahhh"*)**.
4. **`FLUTE` (Flauta de Fantasía)**:
   - Ataque con un ligero soplo de aire dulce y cuerpo limpio (cruce entre flauta travesera y ocarina), ideal para melodías pastorales o aventureras.

#### Bloque 2: Melodía, Base y Acústicos
5. **`PIANO` (Piano Cálido / Rhodes)**:
   - Ataque claro de martillo y caída natural agradable; el instrumento más neutro y cómodo para sacar canciones de oído.
6. **`PLUCK` (Arpa / Cuerda Pulsada)**:
   - Sonido limpio con caída rápida y resonancia bonita para arpegios y acompañamientos rítmicos.
7. **`BASS` (Bajo Sólido)**:
   - Diseñado específicamente para el pequeño altavoz de la DSi: con armónicos medios-graves claros para que la nota se entienda perfectamente en las octavas `2` y `3` sin barrer al resto de pistas.

#### Bloque 3: Chiptune Expresivo (8-Bit con Envolvente)
8. **`CHIP LEAD` (Onda Pulso con Envolvente)**:
   - Onda de consola clásica pero gobernada por una envolvente de volumen por software en cada frame (ataque con pegada y caída gradual suave), para que suene a videojuego clásico de calidad y no a pitido estático.
9. **`CHIP WAVE` (Onda Triangular / 12% Suave)**:
   - Timbre retro más hueco y redondo, perfecto para contramelodías 8-bit o bajos chiptune.

#### Bloque 4: Ritmo y Percusión
10. **`DRUMS` (Kit de Percusión Épico + Retro en el teclado `0..11`)**:
    - En lugar de cambiar de altura una misma nota, al elegir `DRUMS` las 12 teclas disparan golpes de percusión reales pensados para bandas sonoras y chiptune:
      - **Teclas `0..3` (Graves)**: Bombo profundo (`0`), Timbal de cine grave (`1`), Timbal de cine medio (`2`), Timbal de cine alto (`3`).
      - **Teclas `4..7` (Medios)**: Caja limpia (`4`), Caja épica/redoble (`5`), Palmas/Rimshot (`6`), Tom (`7`).
      - **Teclas `8..11` (Agudos)**: Hi-hat cerrado (`8`), Hi-hat abierto (`9`), Platillo/Crash épico (`10`), Campana/Chime (`11`).

---

## 5. Aprender a Componer Jugando (El "Manual del Pequeño Compositor")

El juego no impone teoría musical en los menús, pero te da las herramientas visuales para descubrirla y viene acompañado de un manual práctico pensado para componer con los números `0..11`:

1. **Compás visual (`4/4` vs `3/4`)**:
   - Si la canción que tienes en la cabeza suena a marcha, pop, rock o himno cuadrado (*Star Wars*, *Indiana Jones*, *Mario*), usas **`4/4`** (grupos de 4 pulsos en la cebra del pentagrama).
   - Si suena a balanceo, vals o fantasía medieval (*Juego de Tronos*, *Zelda Ocarina*, *Harry Potter*), cambias a **`3/4`** (grupos de 3 pulsos en la cebra).
2. **Sombra de Pistas + La Regla del `+0`, `+3/+4` y `+7`**:
   - Mientras editas una pista de acompañamiento (ej. `BASS`, `CHOIR` o `STRINGS`), ves en el pentagrama la **sombra gris** de la melodía principal y qué número (`0..11`) está sonando arriba.
   - Con ese número a la vista, tú decides qué emoción darle sumando casillas:
     - **`+0` (El Muro)**: Poner el mismo número una o dos octavas más abajo da peso inmediato sin cambiar la emoción.
     - **`+7` (El Escudo Épico)**: Sumar 7 casillas (ej. de `0` a `7`, o de `2` a `9`, restando 12 si te pasas de 11) crea una armonía de cine potente y abierta que nunca choca.
     - **`+3` (Sombra / Drama)** vs **`+4` (Luz / Triunfo)**: Sumar 3 casillas viste la nota de melancolía o seriedad; sumar 4 casillas la viste de alegría o victoria heroica.

---

## 6. Arranque y Biblioteca (`CONTINUE / NEW`)

- Se mantiene el arranque actual con **`CONTINUE`** (biblioteca de hasta 16 canciones con sus etiquetas dibujadas a mano) y **`NEW`** (`DRAW A NAME` con trazo continuo), entrando desde ahí directamente al Compositor de 2 niveles.
- Guardado automático y transparente en tarjeta SD (`DSSG`) tras cada edición.

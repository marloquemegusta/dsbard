# Manual de DS Stylophone / Looper

## 1. Qué es el juego

DS Stylophone / Looper es un instrumento musical para Nintendo DS y DSi. La pantalla táctil funciona como un teclado cromático de 12 franjas numeradas, mientras los botones permiten cambiar de octava, elegir timbre y construir hasta tres loops independientes.

La aplicación no es un juego con niveles ni puntuación: su objetivo es tocar, grabar y combinar frases musicales.

## 2. Pantalla y conceptos principales

- **Franjas táctiles `0`–`11`**: cada franja produce una nota cromática. Las franjas tienen el mismo ancho y sus etiquetas son siempre numéricas; no se muestran nombres de notas.
- **Octava**: determina la altura de las franjas. Se puede seleccionar de la 2 a la 6; la octava inicial es la 4.
- **Track**: hay tres pistas independientes (`1`, `2` y `3`). Una pista puede contener sus propios eventos, duración, timbre y estado de mute.
- **Timbre**: es el sonido usado para las notas tocadas en directo y para las nuevas notas grabadas. Cada evento grabado conserva el timbre que tenía al grabarse.
- **Transporte**: reloj común que reproduce los loops y puede pausarse.
- **Diario**: registro temporal de las franjas tocadas. Guarda literalmente valores `0`–`11`, sin incluir octava, MIDI ni frecuencia.

La pantalla muestra el estado de la pista seleccionada, la octava, el timbre y la información musical disponible durante la interpretación. El diario y la ayuda son elementos temporales de la sesión.

## 3. Tocar

Toca una franja para producir su nota. También puedes tocar y arrastrar el stylus: al pasar a otra franja se reproduce la nueva nota. El cambio de zona se reconoce desde el primer frame del toque.

Las doce franjas representan una escala cromática consecutiva. La misma franja cambia de altura al cambiar de octava. El audio conserva las frecuencias musicales en hercios internamente y usa los tonos PCM o PSG según el timbre seleccionado.

## 4. Controles

| Control | Acción |
|---|---|
| Pantalla táctil | Tocar o arrastrar por las franjas `0`–`11`. |
| D-pad arriba | Subir una octava, hasta la 6. |
| D-pad abajo | Bajar una octava, hasta la 2. |
| D-pad izquierda | Seleccionar la pista anterior, pasando de la 1 a la 3 al retroceder. Con `L` pulsado, activa vibrato durante el toque. |
| D-pad derecha | Seleccionar la pista siguiente, pasando de la 3 a la 1 al avanzar. Con `L` pulsado, activa glide entre notas. |
| `A` | Iniciar o detener la grabación de la pista seleccionada. |
| `B` corto | Borrar el último token del diario. |
| `B` mantenido | Limpiar el diario completo al mantenerlo pulsado y liberarlo según el estado indicado por la interfaz. |
| `Y` sin tocar | Insertar un separador de frase en el diario. No requiere tocar una franja. |
| `L` durante un toque | Mantener/sostener la nota mientras dura el toque. |
| `L` + D-pad izquierda | Activar vibrato durante la interpretación. |
| `L` + D-pad derecha | Activar glide entre notas durante la interpretación. |
| `R` + `B` | Limpiar la pista seleccionada. Esta acción elimina sus eventos grabados. |
| `L` + `B` | Deshacer la última regrabación. |
| `START` | Pausar o reanudar el transporte y la reproducción de los loops. |
| `SELECT` corto | Silenciar o activar la pista seleccionada. |
| `SELECT` mantenido | Mostrar ayuda temporal. |
| `X` | Cambiar al siguiente timbre. |

El orden de timbres de `X` es circular:

`RHODES` → `KALIMBA` → `BASS` → `PLUCK` → `GLASS` → `PAD` → `SQUARE` → `PULSE` → `THIN` → `DARK` → `RHODES`.

## 5. Grabar y reproducir loops

La pista `1` es la pista master y define la duración real `T` de la primera grabación. Al pulsar `A` queda armada (`WAIT`), y la primera nota inicia realmente la toma en `t=0`; el silencio posterior a la última nota y hasta el segundo `A` sí forma parte de `T`. Las pistas `2` y `3` no se pueden grabar hasta que exista esa pista master. Al pulsar `A` en ellas quedan armadas (`WAIT`) y esperan la primera nota; esa nota ancla la toma al inicio del ciclo master actual, conservando solo el silencio de esa vuelta antes de tocar. Al pulsar `A` para terminar, su duración se ajusta al múltiplo entero de `T` más cercano (`T`, `2T`, `3T`, etc.). No hay BPM ni cuantización de notas individuales.

1. Elige la pista con izquierda/derecha.
2. Elige la octava con arriba/abajo.
3. Elige el timbre con `X`.
4. Pulsa `A` para comenzar la grabación.
5. Toca las franjas y usa el stylus para interpretar la frase.
6. Pulsa `A` otra vez para terminar la grabación.
7. El loop queda asociado a la pista seleccionada y se reproduce con el transporte.

Las tres pistas son independientes: puedes grabar una base, cambiar de pista y añadir otra línea sin borrar las anteriores. El timbre de cada nota queda guardado en el evento, por lo que una pista puede reproducir notas grabadas con timbres distintos. El timbre actual afecta a las notas nuevas y a la voz tocada en directo.

Una grabación debe tener una duración mínima efectiva de 120 ms. `L+B` permite deshacer la última regrabación y `R+B` borra por completo la pista seleccionada.

`SELECT` permite silenciar una pista sin eliminarla. `START` detiene o continúa el reloj de reproducción; al reanudar, los loops siguen usando el transporte común.

## 6. Diario de franjas

El diario sirve como registro rápido de la interpretación. Cada franja que se pulsa añade su número (`0`–`11`); no registra la octava ni convierte el valor en un nombre de nota. `Y` añade un separador de frase, `B` corto elimina el último token y `B` mantenido vacía todo el diario.

El diario es temporal y no se guarda al cerrar la aplicación. La ayuda que aparece con `SELECT` mantenido también es temporal.

## 7. Timbres

Hay seis timbres PCM multisampleados y cuatro timbres PSG de referencia:

- `RHODES`
- `KALIMBA`
- `BASS`
- `PLUCK`
- `GLASS`
- `PAD`
- `SQUARE`
- `PULSE`
- `THIN`
- `DARK`

Los PCM usan muestras de varias raíces para cubrir el teclado. Los timbres PSG se generan como sonidos sintéticos. Los detalles técnicos de las muestras están en [SAMPLES.md](SAMPLES.md).

## 8. Resumen de una sesión

Una sesión típica consiste en elegir un timbre, tocar una frase, grabarla en una de las tres pistas, seleccionar otra pista y superponer una segunda frase. Después se puede silenciar cada pista, cambiar el timbre de la interpretación en directo, pausar el transporte o limpiar/regrabar una pista.

No se guardan datos entre sesiones: al reiniciar la ROM se pierde el contenido de las pistas y del diario.

## 9. Referencia rápida

`Táctil`: tocar · `↑/↓`: octava · `←/→`: pista · `A`: grabar · `X`: timbre · `START`: transporte · `SELECT`: mute/ayuda · `B`: diario · `L+B`: deshacer regrabación · `R+B`: borrar pista · `Y`: separador.

Este manual está preparado para incorporar imágenes y capturas de pantalla en una revisión posterior.

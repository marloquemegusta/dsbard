# Manual de DS Stylophone / Looper

## 1. Que es el juego

DS Stylophone / Looper es un instrumento musical para Nintendo DS y DSi. La pantalla tactil funciona como un teclado cromatico de 12 franjas numeradas. Los botones permiten cambiar de octava, elegir timbre y construir una cancion con hasta tres pistas.

## 2. Pantalla y conceptos principales

- **Franjas tactiles `0`-`11`**: producen notas cromaticas y siempre muestran numeros. La franja inferior contiene los controles compactos; el timeline aparece en ella al mantener `SELECT`.
- **Octava**: determina la altura de las franjas; se puede seleccionar de la 2 a la 6.
- **Pista**: hay tres pistas independientes. Cada pista contiene una secuencia de frases consecutivas.
- **Frase**: captura una interpretacion con su timing relativo, duracion y timbre por evento. Puede contener silencios al principio y al final.
- **Timeline global**: las tres pistas avanzan sobre el mismo reloj. El transporte puede pausarse, detenerse al final o repetir una selección temporal común a las tres pistas.
- **FOCUS TIMELINE**: mantener `SELECT` intercambia la superficie de trabajo: el teclado queda visible arriba y abajo aparece el editor de notas de la pista seleccionada. `←/→` cambia de pista; no hay playhead ni tiradores de selección globales en esta vista.
- **PULSE GRID**: el control `PULSE` define la duración del pulso en milisegundos. Cada pulso tiene cuatro pasos; las marcas y la corrección manual usan el mismo origen. El metrónomo audible está desactivado: `PULSE` solo sirve como reloj interno.
- **Monitor superior**: cada caja representa una frase completa, incluidos sus silencios. Una frase repetida `xN` aparece como `N` cajas consecutivas.
- **Diario**: registro temporal de las franjas tocadas; guarda valores `0`-`11`.

## 3. Tocar

Toca una franja para producir su nota. Tambien puedes tocar y arrastrar el stylus: al pasar a otra franja se reproduce la nueva nota.

## 4. Controles

| Control | Accion |
|---|---|
| Pantalla tactil | Tocar o arrastrar por las franjas `0`-`11`. |
| D-pad arriba/abajo | Subir o bajar octava. En aceptación, aumentar o reducir repeticiones. |
| D-pad izquierda/derecha | Seleccionar la pista anterior o siguiente. |
| `A` | Abrir/cerrar una captura; en recorte, aceptar la frase. |
| `B` corto | Borrar el ultimo token del diario; en recorte, descartar la captura. |
| `B` mantenido | Limpiar el diario. |
| `Y` | Activar/desactivar `LOOP ON/OFF` al soltarlo. El loop siempre respeta la selección temporal actual. |
| `L` durante un toque | Mantener/sostener la cola de la nota; al grabar, el evento termina al soltar la franja y la cola usa `SUS 1..10`. |
| `L` corto en aceptación | Activar o desactivar el sustain de la nota seleccionada; al activarlo usa el nivel actual de `SUS` y aparece la marca `S`. |
| Panel FX, `PULSE` | Arrastrar el slider para elegir el pulso `250..2000 ms`; el valor por defecto es `1000 ms` y no hace falta configurarlo para tocar o grabar. No emite sonido. |
| Panel FX, `VIB` | Arrastrar el slider para elegir profundidad `0..10`; `0` desactiva el vibrato. |
| Panel FX, `SUS` | Arrastrar el slider para elegir la cola `1..10`; el sustain sigue activo solo mientras se mantiene `L`. |
| `R` + `B` | Borrar todas las frases de la pista seleccionada. |
| `L` + `B` | Deshacer la ultima frase aceptada. |
| `START` | Pausar o reanudar la cancion; si termino sin loop, volver a arrancar desde el inicio de la ventana. En recorte, reproducir o detener la preview. |
| `SELECT` corto | Silenciar o activar la pista seleccionada. |
| `SELECT` mantenido | Activar o cerrar `FOCUS TIMELINE`. |
| `X` | Cambiar al siguiente timbre. |

## 5. Grabar y construir frases

No existe una pista master. Cualquier pista puede ser la primera en grabarse.

1. Selecciona la pista con izquierda/derecha.
2. Pulsa `A`. La pista entra en `WAIT`, pero la captura ya esta abierta.
3. Toca cuando quieras. Se conserva el timing real, incluido el silencio entre `A` y la primera nota.
4. Pulsa `A` para cerrar la captura. Todavia no se guarda.
5. Mantén `SELECT` para entrar en `FOCUS TIMELINE`; el teclado queda arriba y abajo aparece la pista seleccionada con sus notas individuales.
6. Toca el centro de una nota para moverla, o uno de sus bordes para cambiar `START` o `END`. `←/→` permite editar otra pista. Esta vista no usa playhead ni selección global `INICIO..FIN`.
7. Pulsa `START` para reproducir o detener en loop la preview del rango actual. Mientras suena, una barra verde indica la posición actual dentro del rango.
8. En la aceptación, toca y arrastra un bloque de nota: el borde izquierdo edita `START`, el derecho `END` y el centro mueve la nota. Mientras mantienes pulsado, el bloque sigue libremente al stylus; al soltar, se ajusta al cuarto de pulso más cercano. Pulsa `L` corto con una nota seleccionada para activar o desactivar su sustain; la marca `S` indica que está activo. Para recortar la frase, toca las etiquetas finas `S` o `E` de la franja superior y arrastra el límite correspondiente. Los cambios respetan silencios y no solapan notas vecinas.
9. Usa arriba/abajo para elegir el número de repeticiones, mostrado como `xN`, y pulsa `A` para aceptar. La frase se añade inmediatamente después de la última frase de esa pista.
10. Pulsa `B` para descartar la captura sin modificar la pista; la copia original conserva exactamente sus tiempos.

 El recorte desplaza el origen temporal y fija la duracion de la frase. Al cerrar la grabacion, cada inicio y duracion se redondea al cuarto de pulso mas cercano usando `PULSE`; la duracion minima es un cuarto. Los silencios iniciales o finales se conservan solo si quedan dentro del rango elegido.

Una repetición no crea un loop independiente: `x20` coloca veinte copias consecutivas de la misma frase en la pista. En `FOCUS TIMELINE` se editan las notas de la pista, no las cajas de repetición.

`Y` activa o desactiva el loop al soltarlo. En el uso normal el rango es toda la canción; `FOCUS TIMELINE` no cambia ese rango y se dedica a editar notas de la pista seleccionada.

Con el loop desactivado, el transporte se detiene al final de la ventana y deja de sonar. `START` vuelve a arrancar desde el principio de esa ventana. Con el loop activado, al alcanzar el final vuelve automaticamente a su inicio.

Una captura sin notas se descarta directamente. No hay BPM, cuantizacion, sincronizacion automatica entre frases ni dependencia de una pista master.

Mientras se captura una frase nueva, la pista seleccionada se silencia para no mezclar su frase anterior con la interpretacion. Las otras pistas siguen sonando sobre el timeline global, salvo las que esten silenciadas con `SELECT`. Durante la reproduccion, la nota activa de la pista seleccionada se ilumina en verde en el teclado tactil cuando pertenece a la octava visible.

El diario tambien queda congelado durante `WAIT` y `REC`, para poder leer la frase anterior mientras se repite o se transforma en una nueva captura.

## 6. Diario de franjas

Cada franja pulsada anade su numero (`0`-`11`). `B` corto elimina el ultimo token y `B` mantenido vacia el diario. El diario no incluye separadores, es temporal y no registra las notas producidas por los loops.

## 7. Timbres

Los timbres disponibles son:

`RHODES`, `KALIMBA`, `BASS`, `PLUCK`, `GLASS`, `PAD`, `SQUARE`, `PULSE`, `THIN`, `DARK`.

Cada evento conserva el timbre que tenia al grabarse.

## 8. Resumen de una sesion

Una sesion tipica consiste en construir poco a poco una linea en cada una de las tres pistas. Cada frase se captura libremente, se recorta, se escucha en preview y se acepta en serie. Despues se pueden silenciar pistas, pausar el transporte, deshacer la ultima frase o borrar una pista completa.

No se guardan datos entre sesiones.

## 9. Referencia rapida

`Tactil`: tocar, sliders o timeline · `↑/↓`: octava o repeticiones en aceptación · `←/→`: pista · `A`: captura/aceptar · `B`: diario o cancelar · `Y`: loop ON/OFF · `START`: transporte/preview · `X`: timbre · `SELECT`: mute o FOCUS TIMELINE · `L+B`: deshacer frase · `R+B`: borrar pista.

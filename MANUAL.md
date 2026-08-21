# Manual de DS Stylophone / Looper

## 1. Que es el juego

DS Stylophone / Looper es un instrumento musical para Nintendo DS y DSi. La pantalla tactil funciona como un teclado cromatico de 12 franjas numeradas. Los botones permiten cambiar de octava, elegir timbre y construir una cancion con hasta tres pistas.

## 2. Pantalla y conceptos principales

- **Franjas tactiles `0`-`11`**: producen notas cromaticas y siempre muestran numeros. La franja inferior contiene los controles compactos; el timeline aparece en ella al mantener `SELECT`.
- **Octava**: determina la altura de las franjas; se puede seleccionar de la 2 a la 6.
- **Pista**: hay tres pistas independientes. El flujo antiguo conserva frases; la exploración de compositor también puede añadir `NoteEvent` individuales directamente al ciclo global.
- **Frase**: captura una interpretacion con su timing relativo, duracion y timbre por evento. Puede contener silencios al principio y al final.
- **Timeline global**: las tres pistas avanzan sobre el mismo reloj. El transporte puede pausarse, detenerse al final o repetir una selección temporal común a las tres pistas.
- **SELECT / selector de frases**: mantener `SELECT` intercambia la superficie de trabajo: el teclado queda visible arriba y abajo aparecen las tres pistas con la misma escala temporal y las mismas cajas de frase que el monitor superior. Cada repetición aparece como una caja consecutiva. Toca una caja para abrir el mismo editor de notas usado durante la aceptación; no hay playhead ni selección global.
- **PULSE GRID**: el control `PULSE` define el periodo real del reloj global en milisegundos (`250..2000`). Cada marca emite un pitido breve de 100 ms cuando `R` está activo. `A` no reinicia el reloj: grabar empieza dentro del pulso que esté sonando. Cada pulso tiene cuatro pasos; las marcas y la corrección manual usan el mismo origen. El clic no se guarda como nota.
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
| `A` | Abrir/cerrar una captura; en recorte, aceptar la frase o confirmar experimentalmente una nota individual. |
| `B` corto | Borrar el ultimo token del diario; en recorte, descartar la captura o cancelar una inserción individual sin residuo. |
| `B` mantenido | Limpiar el diario. |
| `Y` | Activar/desactivar `LOOP ON/OFF` al soltarlo. El loop siempre respeta la selección temporal actual. |
| `L` durante un toque | Mantener/sostener la cola de la nota; al grabar, el evento termina al soltar la franja y la cola usa `SUS 1..10`. |
| `L` corto en aceptación | Activar o desactivar el sustain de la nota seleccionada; al activarlo usa el nivel actual de `SUS` y aparece la marca `S`. |
| Panel FX, `PULSE` | Arrastrar el slider para elegir el periodo global `250..2000 ms`; el valor por defecto es `1000 ms`. No hace falta configurarlo para tocar o grabar. `R` silencia/des-silencia el click sin reiniciar el reloj. |
| Panel FX, `VIB` | Arrastrar el slider para elegir profundidad `0..10`; `0` desactiva el vibrato. |
| Panel FX, `SUS` | Arrastrar el slider para elegir la cola `1..10`; el sustain sigue activo solo mientras se mantiene `L`. |
| `R` + `B` | Borrar todas las frases de la pista seleccionada. |
| `L` + `B` | Deshacer la ultima frase aceptada. |
| `START` | Pausar o reanudar la cancion; si termino sin loop, volver a arrancar desde el inicio de la ventana. En recorte, reproducir o detener la preview. |
| `SELECT` corto | Silenciar o activar la pista seleccionada. |
| `SELECT` mantenido | Abrir o cerrar el selector de frases. |
| `X` | Cambiar al siguiente timbre. |

## 5. Grabar y construir frases

### Compositor experimental de notas individuales

1. Toca una franja fuera del selector para preescucharla; esto no cambia ninguna pista.
2. Mantén `SELECT` para mostrar el timeline y toca un hueco de la fila de la pista seleccionada. Se abre `INSERT NOTE` con una nota provisional usando la última franja preescuchada.
3. Arrastra el centro de la nota para moverla y sus bordes para cambiar `START` o `END`. `START` reproduce la preview del ciclo global.
4. Pulsa `A` para colocar el `NoteEvent` en la pista o `B` para cancelarlo. Mientras aparece `INSERT NOTE`, la composición todavía no ha cambiado.

Esta confirmación `A/B` es deliberadamente experimental para esta ronda. El siguiente revisor debe intentar eliminarla y conservar el gesto directo si resulta suficientemente claro y cancelable.

El compositor no migra frases ni permite todavía zoom, polifonía, overdub o persistencia. Las frases anteriores siguen usando el flujo de captura y aceptación descrito abajo.

No existe una pista master. Cualquier pista puede ser la primera en grabarse.

1. Selecciona la pista con izquierda/derecha.
2. Pulsa `A`. La pista entra en `WAIT`, pero la captura ya esta abierta.
3. Toca cuando quieras. Se conserva el timing real, incluido el silencio entre `A` y la primera nota.
4. Pulsa `A` para cerrar la captura. Todavia no se guarda.
5. Mantén `SELECT` para abrir el selector de frases; el teclado queda arriba y abajo aparecen las tres pistas con la misma representación temporal del monitor superior.
6. Toca una caja concreta. Se abre el mismo editor de aceptación, ahora con una copia de esa frase.
7. Toca el centro de una nota para moverla, o uno de sus bordes para cambiar `START` o `END`. Mientras mantienes pulsado, el bloque sigue libremente al stylus; al soltar, se ajusta al cuarto de pulso más cercano.
8. Pulsa `L` corto con una nota seleccionada para activar o desactivar su sustain; la marca `S` indica que está activo. Ajusta también los límites `S` y `E` de la frase si hace falta.
9. Pulsa `START` para reproducir o detener la preview. Pulsa `A` para reemplazar la frase editada; sus repeticiones conservan el mismo contenido actualizado. Pulsa `B` para cancelar y volver al selector sin modificarla.
10. Pulsa `B` otra vez en el selector para cerrarlo. Las frases posteriores se recolocan si la duración de la frase editada ha cambiado.

El recorte fija la duracion de la frase en pulsos completos: al cerrar la grabacion, el ciclo se redondea hacia arriba a `N x PULSE` y conserva los silencios iniciales o finales. Los inicios y duraciones de las notas se redondean al cuarto de pulso; los tiradores `START` y `END` se ajustan a pulsos completos para que las repeticiones no deriven.

Una repetición no crea un loop independiente: `x20` coloca veinte copias consecutivas de la misma frase en la pista. El selector edita la fuente y actualiza todas sus repeticiones.

`Y` activa o desactiva el loop al soltarlo. En el uso normal el rango es toda la canción; el selector de frases no cambia ese rango ni el transporte.

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

`Tactil`: tocar, sliders o selector/editor de frases · `↑/↓`: octava o repeticiones en aceptación · `←/→`: pista · `A`: captura/aceptar · `B`: diario, cancelar o cerrar selector · `Y`: loop ON/OFF · `START`: transporte/preview · `X`: timbre · `R`: click de pulso ON/OFF · `SELECT`: mute o selector de frases · `L+B`: deshacer frase · `R+B`: borrar pista.

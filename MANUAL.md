# Manual de DS Stylophone / Looper

## 1. Que es el juego

DS Stylophone / Looper es un instrumento musical para Nintendo DS y DSi. La pantalla tactil funciona como un teclado cromatico de 12 franjas numeradas. Los botones permiten cambiar de octava, elegir timbre y construir una cancion con hasta tres pistas.

## 2. Pantalla y conceptos principales

- **Franjas tactiles `0`-`11`**: producen notas cromaticas y siempre muestran numeros.
- **Octava**: determina la altura de las franjas; se puede seleccionar de la 2 a la 6.
- **Pista**: hay tres pistas independientes. Cada pista contiene una secuencia de frases consecutivas.
- **Frase**: captura una interpretacion con su timing relativo, duracion y timbre por evento. Puede contener silencios al principio y al final.
- **Timeline global**: las tres pistas avanzan sobre el mismo reloj. La cancion completa vuelve a empezar al terminar la pista mas larga.
- **Monitor superior**: cada caja representa una frase completa, incluidos sus silencios. Una frase repetida `xN` aparece como `N` cajas consecutivas.
- **Diario**: registro temporal de las franjas tocadas; guarda valores `0`-`11`.

## 3. Tocar

Toca una franja para producir su nota. Tambien puedes tocar y arrastrar el stylus: al pasar a otra franja se reproduce la nueva nota.

## 4. Controles

| Control | Accion |
|---|---|
| Pantalla tactil | Tocar o arrastrar por las franjas `0`-`11`. |
| D-pad arriba/abajo | Subir o bajar octava. En recorte, aumentar o reducir repeticiones. |
| D-pad izquierda/derecha | Seleccionar la pista anterior o siguiente. |
| `A` | Abrir/cerrar una captura; en recorte, aceptar la frase. |
| `B` corto | Borrar el ultimo token del diario; en recorte, descartar la captura. |
| `B` mantenido | Limpiar el diario. |
| `Y` | Insertar un separador de frase en el diario. |
| `L` durante un toque | Mantener/sostener la nota. |
| `L` + D-pad izquierda | Vibrato durante la interpretacion. |
| `L` + D-pad derecha | Glide entre notas. |
| `R` + `B` | Borrar todas las frases de la pista seleccionada. |
| `L` + `B` | Deshacer la ultima frase aceptada. |
| `START` | Pausar o reanudar la cancion; en recorte, reproducir o detener la preview. |
| `SELECT` corto | Silenciar o activar la pista seleccionada. |
| `SELECT` mantenido | Mostrar ayuda temporal. |
| `X` | Cambiar al siguiente timbre. |

## 5. Grabar y construir frases

No existe una pista master. Cualquier pista puede ser la primera en grabarse.

1. Selecciona la pista con izquierda/derecha.
2. Pulsa `A`. La pista entra en `WAIT`, pero la captura ya esta abierta.
3. Toca cuando quieras. Se conserva el timing real, incluido el silencio entre `A` y la primera nota.
4. Pulsa `A` para cerrar la captura. Todavia no se guarda.
5. Aparece la franja de recorte en la parte inferior de la pantalla tactil. El piano sigue visible arriba.
6. Arrastra `START` y `END` con el stylus. Por defecto abarcan desde el comienzo de la primera nota hasta el final de la ultima.
7. Pulsa `START` para reproducir o detener en loop la preview del rango actual.
8. Usa arriba/abajo para elegir el numero de repeticiones, mostrado como `xN`.
9. Pulsa `A` para aceptar. La frase se anade inmediatamente despues de la ultima frase de esa pista.
10. Pulsa `B` para descartar la captura sin modificar la pista.

El recorte desplaza el origen temporal y fija la duracion de la frase. No cuantiza ni modifica el timing interno. Los silencios iniciales o finales se conservan solo si quedan dentro del rango elegido.

Una repeticion no crea un loop independiente: `x20` coloca veinte copias consecutivas de la misma frase en la pista. Las pistas mas cortas quedan en silencio mientras continua el timeline global. Al terminar la pista mas larga, la cancion completa vuelve a empezar.

Una captura sin notas se descarta directamente. No hay BPM, cuantizacion, sincronizacion automatica entre frases ni dependencia de una pista master.

## 6. Diario de franjas

Cada franja pulsada anade su numero (`0`-`11`). `Y` anade un separador, `B` corto elimina el ultimo token y `B` mantenido vacia el diario. El diario es temporal.

## 7. Timbres

Los timbres disponibles son:

`RHODES`, `KALIMBA`, `BASS`, `PLUCK`, `GLASS`, `PAD`, `SQUARE`, `PULSE`, `THIN`, `DARK`.

Cada evento conserva el timbre que tenia al grabarse.

## 8. Resumen de una sesion

Una sesion tipica consiste en construir poco a poco una linea en cada una de las tres pistas. Cada frase se captura libremente, se recorta, se escucha en preview y se acepta en serie. Despues se pueden silenciar pistas, pausar el transporte, deshacer la ultima frase o borrar una pista completa.

No se guardan datos entre sesiones.

## 9. Referencia rapida

`Tactil`: tocar · `↑/↓`: octava o repeticiones en recorte · `←/→`: pista · `A`: captura/aceptar · `B`: diario o cancelar · `START`: transporte/preview · `X`: timbre · `SELECT`: mute/ayuda · `L+B`: deshacer frase · `R+B`: borrar pista.

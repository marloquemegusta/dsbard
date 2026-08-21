# ADN de diseño del instrumento musical para Nintendo DSi / NDS

> **Documento de contexto y criterio para Codex**
>
> Este archivo no pretende ser una especificación cerrada ni un backlog. Su función es volcar el criterio de diseño acumulado durante las conversaciones sobre el instrumento: qué se intenta construir, qué sensación debería producir, qué problemas se han descubierto al usarlo, qué soluciones encajan o no encajan con su filosofía y cómo han evolucionado algunas decisiones.
>
> La idea es que este documento sirva como material de alta entropía del que después pueda destilarse un `AJUSTES.md`, `DESIGN.md` o equivalente mucho más corto y operativo, idealmente contrastándolo además con el estado real del repositorio.
>
> **Regla de lectura importante:** cuando este documento describa una idea antigua y una decisión posterior que la sustituye, la decisión posterior representa mejor el criterio actual. Las ideas antiguas se conservan porque explican por qué se llegó al estado actual.

---

# 1. Qué es realmente este proyecto

Este proyecto no nace de querer hacer un DAW para Nintendo DS, ni un secuenciador completo, ni un sintetizador técnicamente impresionante.

Nace de querer convertir una Nintendo DSi en **un instrumento musical pequeño, inmediato, juguetón y personal**.

La referencia mental más útil es algo entre:

- un Stylophone;
- un pequeño teclado o xilófono electrónico;
- una loopbox muy reducida;
- un cuaderno para recordar frases musicales;
- un juguete musical de Nintendo;
- y, a largo plazo, un instrumento dedicado que simplemente se enciende y se toca.

La consola importa porque combina:

- dos pantallas;
- una pantalla táctil;
- botones físicos;
- tamaño portátil;
- altavoces integrados;
- batería;
- estética de juguete/instrumento;
- hardware limitado que obliga a simplificar.

La limitación no se considera únicamente un problema técnico. También es una virtud de diseño.

El instrumento debería tener una identidad suficientemente fuerte como para que **usar una DSi concreta para esto tenga sentido aunque existan herramientas musicales muchísimo más potentes en PC o móvil**.

---

# 2. Propósito central

El objetivo más profundo no es producir canciones terminadas.

Es **tocar**.

Más concretamente:

1. abrir el instrumento;
2. buscar una melodía de oído;
3. descubrir los saltos entre sonidos;
4. repetirla hasta interiorizarla;
5. recordar pequeñas frases;
6. improvisar;
7. construir capas sencillas;
8. dejar que una idea musical crezca jugando con ella.

El instrumento debe favorecer una relación muy directa:

**oigo algo → lo busco → lo toco → lo repito → lo recuerdo → lo combino.**

Esto conecta con un objetivo personal más amplio: mejorar la capacidad de **sacar canciones de oído bien y rápidamente en cualquier instrumento**.

Por eso el sistema no debería empujar a pensar primero como productor, compositor de piano roll o editor MIDI.

Debería empujar a pensar como alguien que está escuchando y tocando.

---

# 3. El feeling buscado

Esta sección probablemente sea la más importante de todo el documento.

## 3.1 Abrir y tocar

La aplicación debería sentirse como un aparato.

Idealmente:

**POWER → instrumento.**

No:

**POWER → launcher → proyecto → pista → instrumento → configuración → tocar.**

Durante el desarrollo esto no tiene por qué cumplirse literalmente. Se puede seguir lanzando desde TWiLight Menu++ o usando FTP para iterar.

Pero el destino conceptual sí es importante: cuando el instrumento esté maduro, sería deseable que la DSi pudiera convertirse casi en una unidad dedicada, incluso arrancando directamente la ROM mediante Unlaunch y conservando alguna vía para entrar a mantenimiento/TWiLight.

Esta aspiración resume bastante bien la filosofía general.

## 3.2 Tiene que sentirse como instrumento, no como software

El usuario no debería notar constantemente que está gestionando una aplicación.

Las acciones principales deben corresponder a cosas musicales:

- tocar;
- escuchar;
- grabar;
- repetir;
- silenciar;
- recordar;
- construir una frase.

Cuanto más aparezcan conceptos como:

- proyecto;
- archivo;
- configuración;
- menú;
- pista seleccionada mediante jerarquías;
- diálogo;
- cuantización configurable;
- BPM;
- edición de eventos;
- propiedades;

más fácil es perder el feeling buscado.

No significa que todos esos conceptos estén prohibidos técnicamente.

Significa que no deberían convertirse en el modelo mental principal.

## 3.3 Juguete serio

El tono buscado no es el de una workstation profesional en miniatura.

Debe poder ser limitado, simpático, inmediato, incluso un poco juguetón.

Pero no debería sentirse como una demo o una broma.

La ambición es hacer una cosa pequeña que esté **bien terminada**.

Una herramienta que invite a cogerla durante cinco minutos y acabar tocándola veinte.

## 3.4 Sesiones cortas y exploratorias

La experiencia debe funcionar especialmente bien para sesiones cortas:

- recordar una melodía;
- probar una canción;
- improvisar;
- construir una base;
- experimentar con tres voces;
- volver a una frase.

No debería exigir preparar una sesión.

La preparación mata parte del propósito.

## 3.5 La música se toca antes de programarse

Principio general:

> **Instrumento primero, editor después.**

En caso de duda entre una interacción que deja tocar de manera imperfecta pero inmediata y otra que permite programar exactamente el resultado mediante varios pasos, la primera está más alineada con el proyecto.

Esto no significa rechazar para siempre un secuenciador.

Significa que un secuenciador sería otra herramienta del instrumento, no la definición del instrumento.

---

# 4. Qué NO se quiere construir

Es útil describir explícitamente algunos atractores que podrían desviar el proyecto.

## 4.1 No convertirlo en un DAW

La Nintendo DS no necesita convertirse en Ableton.

No hacen falta por defecto:

- docenas de pistas;
- mixer completo;
- automation lanes;
- piano roll;
- gestión de proyectos;
- edición microscópica de eventos;
- routing;
- plugins;
- buses;
- configuración compleja de tempo;
- workflows de producción.

Cada una de esas cosas puede parecer razonable aislada.

El problema es la suma.

El proyecto gana valor precisamente porque elimina gran parte de eso.

## 4.2 No construir una interfaz alrededor de “modos”

Desde muy temprano apareció una preferencia fuerte por evitar una aplicación organizada como una colección de modos.

Idealmente tocar siempre debería seguir siendo tocar.

Puede haber estados internos —grabando, preescuchando, reproduciendo, muteado—, pero deberían corresponder a acciones naturales y visibles, no a navegar por una máquina de estados abstracta.

## 4.3 No obligar a configurar antes de hacer música

Evitar como requisito inicial:

- elegir BPM;
- elegir compás;
- elegir escala;
- elegir proyecto;
- elegir duración;
- elegir instrumento mediante un menú profundo;
- preparar una pista.

La configuración debería surgir solo cuando resuelve un problema real detectado tocando.

## 4.4 No perseguir complejidad por “ser estándar”

Que una funcionalidad exista en todos los loopers comerciales no implica que deba existir aquí.

Las convenciones estándar son útiles como fuente de soluciones, no como checklist.

La pregunta relevante es:

> ¿esta función mejora la experiencia concreta de este instrumento más de lo que aumenta su carga mental?

---

# 5. Pensar en intervalos, no en nombres de nota

Esta es una decisión conceptual importante.

El teclado actual trabaja con **12 identificadores relativos, 0–11**.

La lógica futura del diario musical, frases y secuenciador debería expresarse principalmente en esos IDs relativos, no en nombres absolutos como C, D, E, etc.

Esto no es solo una simplificación de UI.

Está conectado con el objetivo de aprender a tocar de oído.

## 5.1 Razón pedagógica

Cuando se intenta sacar una canción de oído, interesa aprender:

- cuánto sube una nota respecto a la anterior;
- cuánto baja;
- qué patrones se repiten;
- qué forma tiene la frase;
- cómo encontrar la primera nota;
- cómo transportar mentalmente la relación a otro instrumento o registro.

Pensar primero en “Do sostenido” puede fijar una melodía a una posición absoluta.

Pensar en relaciones ayuda a adquirir una representación transferible.

## 5.2 El instrumento como entrenador informal del oído

No se pretende convertir la aplicación en una app de ear training con ejercicios, puntuaciones y tests.

La idea es más orgánica.

La propia manera de representar lo que se toca puede favorecer que el usuario empiece a reconocer patrones.

Por eso el diario puede tener un valor enorme aunque técnicamente sea muy sencillo.

---

# 6. El teclado

La pantalla inferior es, ante todo, **el instrumento**.

La preferencia visual se ha desplazado hacia una representación más cercana a un piano que a una rejilla abstracta, porque el piano da una geografía musical intuitiva y reconocible.

Aun así, internamente la identidad principal puede seguir siendo 0–11.

El teclado debería dar prioridad absoluta a:

- targets táctiles cómodos;
- respuesta inmediata;
- legibilidad;
- relación espacial estable;
- mínimo ruido visual.

La pantalla inferior no debería terminar ocupada por controles secundarios hasta hacer incómodo tocar.

---

# 7. Octavas y rango

Una decisión temprana fue diferenciar claramente:

- **cambio de octava: útil y necesario**;
- **transposición global: no necesaria como concepto principal**.

El cambio de octava responde a una limitación física del rango disponible en pantalla.

No cambia la lógica relativa de la melodía.

La transposición introduce un concepto adicional que no aporta demasiado al núcleo inicial.

---

# 8. Sonido: no resignarse al chiptune

Otra preocupación recurrente fue que el instrumento pudiera sonar musicalmente agradable y no únicamente como un juguete de ondas cuadradas.

La intención es probar aproximadamente 5–6 instrumentos con personalidades diferentes y elegir mediante uso cuáles funcionan.

La meta no es un motor de síntesis general.

Es una colección pequeña de voces con las que apetezca tocar.

Criterios importantes:

- respuesta rápida;
- ataque musical;
- buena inteligibilidad por los altavoces pequeños;
- sonidos diferenciables;
- suficientemente agradables para tocar melodías;
- evitar que todos parezcan variaciones del mismo beep.

El hardware puede imponer compromiso en sample rate, memoria y polifonía.

Es preferible tener unos pocos sonidos con carácter y buen comportamiento a una biblioteca enorme mediocre.

---

# 9. Pistas y loops: intención original

Una de las primeras ideas centrales del instrumento fue poder **grabar y superponer pequeñas capas**.

La formulación inicial fue aproximadamente:

- hasta 3 pistas;
- independientes;
- grabar / reproducir / pausar;
- construir una base, una voz intermedia y una melodía;
- sin overdub dentro de la misma pista inicialmente;
- posibilidad de silenciar pistas;
- acción para controlar/reproducir el conjunto.

La ausencia inicial de overdub es deliberada.

El overdub es cómodo en un looper convencional, pero también vuelve más difícil separar mentalmente qué contiene cada capa.

Tres pistas explícitas son suficientemente potentes para jugar y suficientemente sencillas para entender.

---

# 10. Evolución de la duración de los loops

Aquí hubo una evolución importante.

## 10.1 Primera idea: duraciones totalmente independientes

Al principio resultaba atractivo que cada una de las tres pistas pudiera tener su propia duración y repetir independientemente.

Esto permitía polirritmos y superposiciones emergentes sin configuración.

Conceptualmente era elegante.

Sin embargo, al usar el sistema aparecieron problemas de sincronización y control.

## 10.2 Problema: libertad temporal no equivale a facilidad musical

Si cada loop empieza y termina exactamente cuando se pulsa el botón, crear una base periódica exige una precisión manual que no es divertida.

El problema no es que el músico sea incapaz de mantener un ritmo.

El problema es que se está mezclando:

- tocar una frase;
- decidir dónde empieza el ciclo;
- decidir dónde termina;
- reaccionar físicamente al botón;
- compensar la latencia humana.

La interfaz convierte una acción musical sencilla en una prueba de reflejos.

## 10.3 Idea posterior: pista master y múltiplos

Se exploró entonces un modelo donde una primera pista/master define el ciclo temporal.

Las demás pistas se ajustan a múltiplos enteros de ese ciclo:

- 1 vuelta;
- 2 vueltas;
- 3 vueltas;
- 4 vueltas;
- etc.

La consecuencia importante es que **la primera pista no tiene que ser la más larga**.

Puede ser una célula corta de acompañamiento.

Una melodía posterior puede durar varias vueltas del master.

Ejemplo conceptual:

- master = 4 s;
- pista 2 = 8 s = 2 ciclos;
- pista 3 = 16 s = 4 ciclos.

Esto conserva variedad estructural sin perder sincronización.

## 10.4 Problema posterior: no obligar a componer en orden

Aunque master+múltiplos resuelve una parte del problema, apareció otra limitación:

> si para empezar a construir una melodía necesito haber creado antes una base que defina el master, la herramienta me obliga a componer en un orden concreto.

Eso tampoco encaja del todo.

El instrumento debería permitir empezar por la melodía si eso es lo que se tiene en la cabeza.

Por tanto, **“master” debe entenderse como una solución a la referencia temporal, no como una obligación estética de que la primera voz sea una base**.

La infraestructura temporal no debería dictar qué voz musical se inventa primero.

---

# 11. El problema fundamental de REC: el tiempo de reacción

Este ha sido uno de los descubrimientos de UX más importantes.

Un grabador ingenuo hace esto:

1. pulsas A/REC;
2. empieza el tiempo;
3. tocas;
4. pulsas A/REC;
5. termina el tiempo.

Eso parece directo.

Pero introduce dos errores artificiales:

- cuánto tardas entre pulsar A y tocar la primera nota;
- cuánto tardas entre terminar la última nota y pulsar A.

Esos espacios pueden no formar parte de la música.

Son simplemente tiempo de interacción con la interfaz.

Cuando el loop se repite, los errores se hacen muy evidentes.

## 11.1 Primera postura: conservar silencios explícitos

En una fase anterior se defendió que el grabador no recortara automáticamente:

- REC empieza;
- REC termina;
- los silencios al principio y al final son contenido musical válido.

La motivación era correcta: **el silencio es música** y no debería eliminarse arbitrariamente.

## 11.2 Problema descubierto después

El problema es que, sin una forma de distinguir intención de latencia humana, el sistema interpreta como música algo que muchas veces no lo es.

Eso hace extremadamente difícil grabar una vuelta periódica limpia.

Por tanto, el criterio evolucionó.

La conclusión actual no es “eliminar silencios”.

Es:

> **preservar silencios musicales sin obligar al usuario a ejecutar con precisión los silencios administrativos producidos por pulsar REC.**

Esta formulación es mucho más útil que cualquiera de las reglas anteriores por separado.

---

# 12. Grabación esperando la primera nota

Una solución especialmente alineada con el problema es que pulsar A arme la grabación, pero que **el contenido temporal real empiece con la primera nota**.

Así:

1. A arma;
2. el músico se coloca;
3. la primera nota establece el comienzo musical.

Esto elimina el tiempo de reacción inicial.

Además tiene buen feeling: pulsar REC significa “estoy preparado”, no “el reloj ha empezado y ya voy tarde”.

---

# 13. ¿Y el silencio antes de la primera nota?

Aquí hay una sutileza importante.

Si siempre recortamos hasta la primera nota, parecería imposible crear una frase cuyo comienzo musical incluya silencio.

La solución conceptual discutida fue no definir ese silencio respecto al instante de pulsar A.

En un contexto cíclico puede definirse respecto a la vuelta anterior o respecto a una referencia temporal conocida.

Es decir:

- A no debería ser una marca musical de alta precisión;
- las notas sí son eventos musicales;
- el sistema debería inferir o permitir expresar la colocación de la frase dentro de un ciclo sin depender del reflejo al pulsar A.

Esta idea todavía puede tener detalles de implementación abiertos.

Lo importante es el principio.

---

# 14. Frases como abstracción intermedia

Una evolución importante ha sido empezar a pensar en **frases**.

“Frase” parece encajar mejor que “clip MIDI”, “take”, “region” o “pattern” con la forma de usar el instrumento.

Una frase es algo que se toca, se escucha y se recuerda.

## 14.1 Flujo explorado

Se planteó un flujo parecido a:

1. empezar a grabar;
2. tocar una frase;
3. terminar;
4. escuchar inmediatamente esa frase repetida;
5. decidir cuántas vueltas/ciclos ocupa o cómo fijarla;
6. confirmar o cancelar.

Los controles discutidos incluían:

- `A`: confirmar;
- `B`: cancelar;
- flechas: cambiar número de copias/vueltas;
- preescucha en loop antes de fijar.

El detalle exacto puede cambiar con la implementación.

El valor de este flujo está en otra cosa:

> entre tocar algo y comprometerlo a la estructura existe un pequeño espacio de escucha.

Eso permite corregir problemas estructurales sin introducir un editor.

---

# 15. Preescucha: corregir con el oído, no con una cuadrícula

La preescucha encaja especialmente bien con el proyecto porque usa la capacidad musical del usuario para juzgar el resultado.

En vez de enseñar:

- 3.73 segundos;
- 932 samples;
- una rejilla;
- un punto de corte;

el instrumento reproduce.

El usuario oye si “cae” bien.

Esto es coherente con toda la filosofía de tocar de oído.

La interfaz debería intentar resolver problemas musicales mediante escucha siempre que sea razonable.

---

# 16. Explorar una voz sin construir obligatoriamente toda la canción

Apareció también una distinción útil entre dos actividades, aunque no necesariamente deba manifestarse como dos “modos” explícitos.

## Exploración

Estoy intentando encontrar una melodía/frase.

Puedo:

- tocar;
- equivocarme;
- repetir;
- guardar mentalmente secuencias;
- escuchar las últimas frases;
- desarrollar una voz sola.

## Construcción / reproducción

Ya tengo material y quiero oír varias capas juntas.

Esta distinción es importante porque un looper demasiado centrado en “la canción global” puede entorpecer el primer caso.

No debería ser obligatorio tener una base activa para trabajar cómodamente una melodía.

---

# 17. Reproducción de frases recientes

Se discutieron alternativas para revisar lo tocado.

Una idea especialmente alineada con memorización es poder reproducir **las últimas N frases**, en secuencia, sin necesariamente convertirlas en un loop infinito.

Esto sirve para recordar cómo continuaba una melodía.

También se habló de un reinicio muy fácil mediante `Start`.

El criterio aquí es que la reproducción no solo sirve para “performance”.

También sirve como memoria externa temporal.

---

# 18. Diario musical

El diario de notas/intervalos es una de las funcionalidades con mayor potencial porque conecta directamente la aplicación con el aprendizaje de oído.

La idea base:

- mientras se toca, se registra una secuencia sencilla;
- se representa usando IDs 0–11;
- se puede consultar para recordar qué se acaba de tocar;
- debería ser fácil borrar al menos la última entrada;
- su función principal es **recordar**, no editar una canción.

## 18.1 El diario no debe convertirse automáticamente en secuenciador

Esto fue una distinción explícita.

Que una nota aparezca en el diario no significa que pase a formar parte de una pista programada.

El diario es una ayuda cognitiva.

Puede convertirse algún día en material reutilizable, pero esa no debería ser su semántica inicial.

## 18.2 Por qué puede ser más valioso que features más espectaculares

El problema que realmente aparece al sacar canciones de oído no siempre es “no encuentro la nota”.

A menudo es:

- encuentro cuatro notas;
- sigo buscando;
- olvido exactamente las cuatro anteriores;
- tengo que empezar otra vez.

Una memoria visual mínima resuelve ese problema sin cambiar la manera de tocar.

Es un gran ejemplo del tipo de feature que encaja en el proyecto:

**poca complejidad de interacción + gran mejora práctica.**

---

# 19. El secuenciador futuro

Existe interés en añadir más adelante un secuenciador simple.

Pero debe preservarse la filosofía del proyecto.

La idea que mejor encaja es un **step sequencer muy reducido basado en nota + duración**, posiblemente alimentado por la misma representación relativa 0–11.

No debería desembocar automáticamente en:

- piano roll completo;
- DAW;
- editor MIDI general;
- decenas de parámetros.

Puede ser útil cuando el usuario quiere construir algo que no le sale interpretando, o fijar una frase exacta.

Pero debería convivir con el tocar, no sustituirlo.

**Prioridad actual: antes de invertir mucho en esto, mejorar memorización, recall y flow de grabación.**

---

# 20. Tres pistas como límite deliberado

El número tres no es simplemente una limitación accidental.

Tres pistas son suficientes para estructuras musicalmente interesantes:

- base;
- acompañamiento;
- melodía;

o:

- percusión;
- bajo;
- lead;

o cualquier combinación.

Pero siguen siendo suficientemente pocas para que la pantalla superior pueda representarlas siempre.

Eso evita una capa de navegación.

Un buen principio sería:

> si una entidad fundamental cabe siempre físicamente en la interfaz, es mejor verla que navegar hacia ella.

---

# 21. Mute y solo

Silenciar pistas manualmente es importante porque permite explorar combinaciones.

No hace falta sofisticarlo demasiado.

La interacción debería responder a preguntas directas:

- ¿cómo suena esto sin la base?
- ¿cómo suena solo la melodía?
- ¿qué pasa si vuelvo a añadir la tercera voz?

Mute/solo tiene mucho sentido si se implementa de forma inmediata.

No debería necesitar abrir mixer.

---

# 22. Pantalla superior: estado musical, no panel de administración

Desde el principio la pantalla superior se imaginó como el lugar donde ver qué está ocurriendo en el tiempo.

Ideas exploradas:

- stack de pistas;
- líneas de actividad/silencio;
- playheads;
- bloques/frases;
- indicación de dónde se encuentra cada loop;
- estructura temporal mínima.

Su función principal es ayudar a **anticipar la sincronización**.

La visualización debería responder visualmente a:

- qué pistas existen;
- cuál está sonando;
- dónde está el playhead;
- qué parte contiene actividad;
- qué frases componen la pista;
- cuándo vuelve a empezar.

No hace falta convertirla en un editor.

La visualización es especialmente valiosa porque el oído oye el presente pero el músico necesita anticipar un poco el futuro.

---

# 23. No añadir visualización por añadir

Cuando se discutió cómo mejorar la sincronización apareció la tentación de añadir nuevos indicadores.

Pero ya existe una visualización temporal.

Por tanto, antes de introducir widgets nuevos conviene aprovechar la representación existente.

Principio extrapolable:

> cuando el problema puede resolverse haciendo más informativa una superficie ya existente, preferirlo a añadir otra superficie conceptual.

---

# 24. Cuantización

La postura general ha sido evitar cuantización obligatoria.

Motivos:

- cambia el feeling de interpretación;
- obliga a introducir BPM/rejilla;
- puede corregir cosas que el usuario no quería corregir;
- acerca el instrumento a un secuenciador.

Sin embargo, la existencia del problema de timing no significa que “todo ajuste temporal” sea malo.

Hay una diferencia importante entre:

## Cuantizar notas

Mover eventos interpretados a una cuadrícula.

y:

## Resolver límites de ciclo

Evitar que el tiempo de reacción al botón REC forme parte accidentalmente del loop.

Lo segundo sí parece necesario.

No debería sacrificarse la usabilidad en nombre de una falsa “pureza” de grabación libre.

---

# 25. Posible nudge temporal

En una fase se sugirió un pequeño `nudge ±` para desplazar ligeramente una pista ya grabada.

Puede ser útil como válvula de escape si dos capas han quedado bien interpretadas pero ligeramente desfasadas.

La idea encaja si:

- es extremadamente simple;
- no abre un editor;
- sirve para corregir un problema audible concreto.

Debe considerarse una herramienta secundaria, no una excusa para mantener un flujo de grabación fundamentalmente incómodo.

---

# 26. Silencio musical vs silencio de interfaz

Esta distinción debería ser casi una regla de arquitectura.

## Silencio musical

El usuario pretende que no haya notas durante un intervalo.

Debe conservarse.

## Silencio de interfaz

El usuario está:

- moviendo el dedo hacia A;
- soltando el stylus;
- reaccionando al final;
- esperando a que el sistema cambie de estado.

No debería convertirse automáticamente en contenido.

Si un algoritmo puede distinguir razonablemente uno de otro mediante contexto, debería intentarlo.

Si no puede, la UI debería permitir expresarlo sin exigir precisión motora absurda.

---

# 27. Latencia cognitiva también importa

No solo importa la latencia de audio.

También importa cuántas decisiones hay entre intención y sonido.

Por ejemplo:

**quiero grabar esto**
→ seleccionar pista
→ elegir longitud
→ elegir vueltas
→ armar
→ esperar
→ grabar

puede ser peor que:

**quiero grabar esto**
→ A
→ tocar.

Aunque ambos tengan la misma latencia técnica.

Cada paso extra debe justificar su existencia.

---

# 28. Botones físicos

Los botones físicos de DS son valiosos porque permiten realizar operaciones estructurales sin robar espacio táctil al teclado.

El criterio general es:

- stylus/pantalla inferior: tocar;
- botones: operaciones discretas;
- evitar controles táctiles minúsculos para transporte/grabación cuando existe un botón físico mejor.

`A`, `B`, `Start`, cruceta y hombros deberían usarse con consistencia.

Si `A` significa confirmar/grabar dentro de un flujo, intentar no convertirlo arbitrariamente en otra cosa dentro del mismo contexto.

---

# 29. Táctil durante decisiones estructurales

En algunos flujos se planteó deshabilitar temporalmente la interacción táctil mientras se decide qué hacer con una frase recién grabada.

Esto puede ser correcto si evita tocar accidentalmente durante una confirmación.

Pero debe mantenerse breve.

La sensación deseada no es entrar en un modo modal largo.

Es más parecido a:

- acabo de tocar algo;
- el instrumento me pregunta mediante feedback inmediato “¿esto?”;
- confirmo/cancelo;
- vuelvo a tocar.

---

# 30. Diseño mediante uso, no por arquitectura imaginada

El desarrollo del proyecto está siendo especialmente útil porque el usuario está ejecutando el instrumento real y descubriendo problemas que no eran evidentes conceptualmente.

Ejemplo paradigmático:

- “conservar silencios de REC” parecía musicalmente correcto;
- al usarlo se descubre que el timing del botón hace imposible crear ciclos agradables;
- se cambia el modelo.

Este método debería mantenerse.

Una feature que parece elegante en abstracto vale menos que una interacción que se siente bien en la DSi.

---

# 31. Calidad de vida antes que amplitud funcional

Cuando se pidió identificar mejoras, aparecieron muchísimas posibilidades.

La reacción correcta fue reducirlas a 2–3 cambios con máximo impacto y poca complejidad.

Ese criterio debería formar parte del proyecto.

Priorizar:

- eliminar una fricción que ocurre cada vez que se graba;
- hacer visible algo que ahora se olvida;
- reducir una operación repetitiva;

antes que:

- añadir un sistema nuevo enorme;
- crear una biblioteca de opciones;
- añadir edición avanzada.

---

# 32. Complejidad presupuestada

Puede ser útil pensar que el instrumento tiene un presupuesto limitado de complejidad.

Cada concepto nuevo gasta parte de él.

Conceptos actuales razonables:

- pista;
- frase;
- vuelta;
- diario;
- instrumento;
- octava;
- mute;
- reproducción.

Si aparecen simultáneamente:

- escena;
- patrón;
- clip;
- proyecto;
- compás;
- swing;
- grid;
- take;
- region;
- automation;

el modelo mental empieza a colapsar.

No significa que la arquitectura interna no pueda usar conceptos técnicos.

Significa que la experiencia pública debería tener muy pocos.

---

# 33. Terminología

Preferir términos cercanos a la experiencia musical.

Buenos candidatos:

- pista;
- frase;
- vuelta;
- tocar;
- grabar;
- repetir;
- diario;
- octava;
- instrumento.

Usar con cuidado:

- clip;
- region;
- take;
- scene;
- pattern;
- quantize;
- timeline editor.

La terminología influye en la forma de diseñar.

Si llamamos “frase” a algo, tendemos a construir una operación pensada para tocar y escuchar.

Si lo llamamos “clip”, tendemos a construir un editor.

---

# 34. Estética

Hay afinidad por una estética de gadget-juguete llamativo, incluso con referencias tipo Frutiger Aero.

No es necesario que cada pantalla siga literalmente una estética concreta.

Lo importante es evitar que la aplicación termine pareciendo una utilidad técnica gris o un DAW encogido.

Puede permitirse:

- personalidad;
- iconografía;
- animación mínima;
- colores;
- feedback juguetón.

Siempre que no sacrifique legibilidad.

---

# 35. Feedback inmediato

Cada acción importante debería producir feedback evidente.

Ejemplos:

- al tocar: sonido inmediato + feedback visual;
- al armar grabación: estado inequívoco;
- al comenzar realmente la frase: feedback;
- al terminar: preescucha;
- al mutear: pista claramente atenuada;
- al borrar: desaparición inmediata;
- al cambiar octava: estado visible.

La DS tiene una pantalla pequeña.

Los estados ambiguos cuestan mucho.

---

# 36. No guardar por defecto puede ser una virtud

En la concepción inicial se aceptaba que los loops fueran temporales.

Eso encaja con la idea de juguete/instrumento.

No todo tiene que convertirse en proyecto persistente.

La ausencia de guardado reduce ansiedad:

- no hay que nombrar;
- no hay que organizar;
- no hay que decidir qué merece conservarse.

A largo plazo quizá tenga sentido guardar algo, especialmente frases o canciones.

Pero la persistencia no debería contaminar el flujo básico.

---

# 37. La DSi como objeto dedicado

El deseo futuro de autoboot tiene una consecuencia importante de diseño.

Si la DSi termina siendo “el instrumento”, la aplicación debería comportarse como firmware de un aparato:

- estable;
- predecible;
- rápido;
- pocos estados rotos;
- recuperación sencilla;
- controles consistentes.

La robustez empieza a importar tanto como añadir features.

---

# 38. Desarrollo con Codex

El desarrollo se realiza de forma muy iterativa con Codex.

El patrón habitual es abordar cambios autocontenidos:

- mejorar X;
- añadir Y;
- probar;
- volver a corregir.

Esto funciona bien y ha permitido llegar a una aplicación que se siente bastante terminada y realmente funcional.

El riesgo es que muchas ideas paralelas aparezcan durante el proceso.

Por eso conviene separar:

## Criterio estable

Principios como:

- abrir y tocar;
- no DAW;
- pensar en intervalos;
- evitar timing administrativo;
- tres pistas;
- instrumento primero.

## Backlog de ideas

Cosas que pueden ser interesantes pero no deben contaminar el núcleo hasta ser necesarias.

La existencia de una idea no implica compromiso de implementación.

---

# 39. Ideas nuevas sin deuda inmediata

Tiene sentido mantener una metodología tipo Kanban ligera para capturar ideas que aparecen durante el desarrollo.

El objetivo no es burocratizar el proyecto.

Es permitir:

- capturar una idea;
- no perderla;
- no implementarla inmediatamente;
- revisarla cuando haya tiempo;
- descartarla sin culpa.

Codex debería respetar esta separación.

Cuando detecte una mejora potencial no debería asumir automáticamente que forma parte del cambio actual.

Scope pequeño y autocontenido por defecto.

---

# 40. Resolver el problema más pequeño que produce el feeling correcto

Ejemplo:

Problema:
el loop queda mal porque se pulsa A tarde.

Solución desalineada:
construir un editor temporal completo.

Solución alineada:
armar grabación y empezar con la primera nota; usar ciclo/frase para decidir el cierre.

El criterio debe buscar la intervención mínima que haga desaparecer la fricción.

---

# 41. Estado conceptual actual del flujo de grabación

No debe interpretarse como especificación exacta del código, pero el modelo actual tiende hacia algo parecido a esto:

1. el usuario decide grabar una frase;
2. A arma la grabación;
3. el sistema no penaliza el tiempo hasta la primera nota;
4. se toca libremente, sin cuantización de notas;
5. A termina la captura/intención;
6. el sistema presenta una preescucha cíclica o temporalmente estructurada;
7. se puede cancelar;
8. se puede aceptar;
9. si hace falta, se determina cuántas vueltas/ciclos representa;
10. se incorpora como frase a la pista;
11. la pantalla superior refleja esa estructura.

Lo importante es que el usuario **toque primero y formalice después**.

---

# 42. Reproducción global y reproducción local

Hay dos necesidades distintas:

## Escuchar la composición

Reproducir las pistas juntas y comprobar la combinación.

## Recordar el material reciente

Escuchar las últimas frases o reiniciar fácilmente una secuencia para reconstruir mentalmente una melodía.

No debería obligarse a usar el mismo mecanismo para ambas cosas si eso vuelve incómoda una de ellas.

La reproducción debe apoyar tanto performance como memoria.

---

# 43. La primera nota como ancla

En muchos flujos, la primera nota tiene una propiedad especial: expresa intención musical con mucha más precisión que la pulsación previa de REC.

Es razonable usarla como ancla temporal.

Pero no debe confundirse esto con eliminar todos los silencios iniciales posibles.

La arquitectura debería dejar abierta la posibilidad de ubicar esa primera nota dentro de un ciclo cuando exista una referencia temporal externa.

---

# 44. El final de la frase

El final es más difícil que el inicio.

La última nota no basta necesariamente para decidir cuándo acaba musicalmente una frase, porque puede existir un silencio final intencionado.

Pero el botón A tampoco debería registrar literalmente toda la latencia humana.

La solución debe apoyarse en estructura:

- referencia de ciclo;
- múltiplos;
- frase;
- preescucha;
- confirmación.

La pregunta correcta no es “¿en qué sample pulsó A?”.

Es “¿qué longitud musical parece estar intentando expresar?”.

---

# 45. Múltiplos enteros como abstracción sencilla

Una de las razones por las que los múltiplos enteros encajan bien es que proporcionan estructura sin introducir unidades técnicas.

“Esta frase dura 2 vueltas” es más musical y manejable que “esta frase dura 7.84 segundos”.

Además:

- facilita sincronización;
- permite melodías largas sobre bases cortas;
- hace predecible el retorno;
- permite visualizar estructura claramente.

Si la implementación actual usa este concepto, debería preservarse mientras siga resolviendo problemas.

---

# 46. Pero evitar la tiranía del master

El valor del master es dar una regla temporal compartida.

No debe convertirse en una jerarquía creativa.

Preguntas que cualquier futura evolución debería pasar:

- ¿puedo empezar por una melodía?
- ¿puedo explorar sin tener todavía una base?
- ¿puedo definir estructura después de tocar?
- ¿el sistema me obliga a decidir demasiado pronto?

Si la respuesta empieza a ser “no”, se está perdiendo parte del propósito.

---

# 47. Sin overdub, al menos de momento

El overdub puede ser atractivo, pero introduce varias ambigüedades:

- ¿cómo deshago una capa concreta?
- ¿qué contiene exactamente la pista?
- ¿cómo borro solo lo último?
- ¿qué longitud tiene la pista?
- ¿qué pasa con las frases?

La arquitectura basada en pocas pistas y frases separadas parece más fácil de razonar.

No hay una prohibición eterna, pero no es prioritario.

---

# 48. Controles / ayuda

Se consideró que `Select` podría mostrar una ayuda breve de controles.

Esto encaja mejor que llenar permanentemente la UI de etiquetas.

La filosofía sería:

- controles principales aprendibles por uso;
- referencia rápida disponible;
- no manual dentro de la pantalla principal.

Una ayuda compacta sirve especialmente durante el desarrollo, cuando los controles todavía pueden cambiar.

---

# 49. Diario y controles: cuidar la pantalla

Cuando se propuso añadir diario y ayuda sobre la interfaz existente, la preocupación principal fue no saturar la pantalla inferior.

El teclado debe conservar prioridad.

Por tanto:

- información secundaria preferentemente arriba o mediante overlay breve;
- controles permanentes solo si son muy frecuentes;
- evitar reducir demasiado las teclas para acomodar UI.

---

# 50. Aprender canciones de oído: cómo debe apoyar el software

El software no debería “resolver” la canción por el usuario.

Debe ayudar en los puntos donde la memoria de trabajo entorpece el aprendizaje.

Ejemplo ideal:

1. busco la primera nota;
2. encuentro una pequeña secuencia;
3. el diario me ayuda a no olvidarla;
4. repito la secuencia;
5. encuentro la continuación;
6. puedo volver a escuchar una frase reciente;
7. gradualmente dependo menos del diario.

El instrumento actúa como andamio, no como respuesta.

---

# 51. Transferencia a otros instrumentos

La representación relativa 0–11 y la práctica de oído deberían servir fuera de la DSi.

El objetivo final no es ser bueno “jugando a esta app”.

Es desarrollar intuición transferible a:

- flauta;
- Stylophone;
- teclado;
- otros instrumentos melódicos.

Por eso debe evitarse introducir representaciones que solo tengan sentido dentro de la UI.

---

# 52. Instrumentos sonoros: criterio de selección

Cuando se implementen o revisen las 5–6 voces, no elegirlas por variedad técnica sino por utilidad tocando.

Preguntas útiles:

- ¿apetece seguir tocando este sonido?
- ¿se distingue bien la altura?
- ¿funciona en los altavoces de DS?
- ¿fatiga?
- ¿tiene suficiente sustain?
- ¿el ataque dificulta melodías rápidas?
- ¿varias voces pueden convivir sin convertirse en barro?
- ¿hay al menos alguna voz cálida/no-chiptune?

La prueba real en hardware debe tener más peso que escucharla en emulador o PC.

---

# 53. Performance sobre hardware real

El hardware objetivo no es un detalle.

La DSi tiene:

- pantalla pequeña;
- altavoces modestos;
- recursos limitados;
- stylus;
- botones físicos;
- latencia y restricciones propias.

Las decisiones deben probarse ahí.

Un diseño que parece perfecto en un mock HTML a tamaño de monitor puede fallar físicamente.

El mock sirve para explorar distribución, no para validar feeling táctil.

---

# 54. La interfaz existente importa

No rediseñar desde cero por sistema.

La aplicación ya tiene una interfaz funcional y una visualización temporal.

Las mejoras deberían intentar integrarse con lo que funciona.

Evitar “feature → nueva pantalla” como respuesta automática.

Muchas funciones pueden expresarse mejor mediante:

- pequeños cambios de estado;
- overlays;
- reutilización de la pantalla superior;
- acciones de botones.

---

# 55. Lo explícitamente descartado o aplazado

A fecha de este documento, no debería asumirse como prioridad:

- un DAW completo;
- un piano roll completo;
- cuantización musical rígida;
- edición compleja de timing;
- visualizaciones nuevas innecesarias;
- overdub sofisticado;
- proyectos/archivos complejos;
- una gran infraestructura de guardado;
- secuenciador avanzado;
- configuración profunda de BPM/compás;
- obligar a empezar por una base;
- nombres de nota absolutos como representación principal.

Algunas de estas ideas pueden volver si resuelven un problema real.

Pero deben justificar su coste desde cero.

---

# 56. Decisiones bastante estables

Estas ideas deberían considerarse parte del ADN salvo evidencia fuerte de que el uso real exige otra cosa:

1. **Instrumento primero.**
2. **Abrir y tocar.**
3. **Tres pistas son suficientes.**
4. **La pantalla inferior pertenece al teclado.**
5. **La superior debe comunicar estructura/estado musical.**
6. **Los botones físicos gestionan acciones estructurales.**
7. **Representación relativa 0–11.**
8. **Diario como memoria, no como secuenciador.**
9. **Cambio de octava.**
10. **No hacer un DAW.**
11. **No cuantizar notas por defecto.**
12. **Eliminar la dependencia del timing preciso de REC.**
13. **Conservar silencios musicales.**
14. **Poder explorar una melodía sin construir antes una base.**
15. **Preferir soluciones pequeñas de gran impacto.**
16. **Probar decisiones en hardware real.**

---

# 57. Decisiones actuales pero revisables

Estas ideas parecen buenas actualmente, aunque son más de implementación que de ADN:

- master + múltiplos enteros;
- frases como unidad de construcción;
- preescucha tras grabar;
- A confirma / B cancela;
- cruceta para seleccionar número de vueltas;
- reproducción de últimas N frases;
- Start como reinicio fácil;
- no overdub;
- ayuda en Select;
- posible nudge temporal.

Codex debería poder modificarlas si encuentra una implementación mejor que preserve los principios superiores.

---

# 58. Preguntas todavía abiertas

No hace falta resolverlas todas ahora.

## 58.1 Referencia temporal sin obligar a crear primero un master

¿Cómo puede existir una rejilla/ciclo compartido sin imponer que la primera voz sea una base?

Posibles direcciones:

- master emergente;
- primer material define referencia pero puede reinterpretarse;
- frase provisional;
- referencia temporal separada de la identidad de pista.

La solución concreta no está cerrada.

## 58.2 Cómo expresar silencio inicial intencionado

Debe poder existir sin depender de esperar con precisión después de REC.

## 58.3 Cómo cerrar una frase

Debe distinguir silencio musical de reacción al botón.

## 58.4 Persistencia

Puede ser útil más adelante, pero se debe decidir qué merece guardarse sin introducir gestión de proyectos.

## 58.5 Secuenciador

Está claro el interés por uno pequeño de nota+duración, pero todavía no debería dominar la arquitectura.

---

# 59. Heurísticas para Codex al proponer cambios

Cuando existan varias soluciones, puntuar mentalmente cada una con estas preguntas.

## A. ¿Reduce pasos entre intención y sonido?

Si sí, buena señal.

## B. ¿Introduce un concepto que el usuario debe aprender?

Si sí, exigir una ganancia clara.

## C. ¿Puede resolverse usando un botón o superficie ya existente?

Preferirlo frente a una pantalla nueva.

## D. ¿Ayuda a tocar o ayuda a administrar?

Priorizar tocar.

## E. ¿El problema ocurre a menudo?

Una fricción pequeña pero constante puede ser prioritaria.

## F. ¿Puede corregirse escuchando en lugar de editando?

Preferir escucha.

## G. ¿Obliga a componer en un orden específico?

Mala señal salvo que sea inevitable.

## H. ¿Funciona en una sesión de 30 segundos?

El núcleo debería hacerlo.

## I. ¿La feature tiene sentido aunque nunca se guarde una canción?

Si sí, probablemente está cerca del espíritu del instrumento.

## J. ¿Lo usaría alguien sin leer el manual?

Si no, simplificar si es una función frecuente.

---

# 60. Heurística para evitar feature creep

Antes de añadir una feature, formular la fricción exacta que resuelve.

Formato recomendado:

> Cuando estoy intentando **X**, ocurre **Y**, lo que me obliga a **Z** y rompe el flow.

Ejemplo:

> Cuando intento grabar una base periódica, el inicio y el final dependen de mi reacción al botón A, lo que introduce silencios accidentales y rompe el loop.

Eso justifica buscar una solución.

En cambio:

> “Los loopers suelen tener cuantización”

no es una necesidad.

---

# 61. Prioridad de diseño actual

El orden conceptual aproximado es:

## Prioridad alta

- feeling al tocar;
- latencia/respuesta;
- grabación cómoda;
- periodicidad que no dependa del reflejo;
- diario/memoria;
- recall de frases;
- claridad visual del estado;
- instrumentos agradables.

## Prioridad media

- refinamiento de frases;
- mute/solo;
- reproducción flexible;
- ayudas de control;
- pequeños ajustes temporales si siguen haciendo falta.

## Prioridad posterior

- secuenciador nota+duración;
- persistencia;
- autoboot como dispositivo final;
- ideas experimentales.

---

# 62. Qué significa “terminado” para este proyecto

No significa tener muchas funciones.

Una versión puede sentirse terminada si:

- se enciende y responde;
- tocar es agradable;
- los controles son consistentes;
- las tres pistas funcionan;
- grabar no da guerra;
- la sincronización es comprensible;
- se puede recordar lo tocado;
- el sonido es suficientemente bueno;
- no hay bugs que obliguen a pensar en el software;
- la UI parece intencionada.

La sensación de producto terminado surge de coherencia y robustez, no del tamaño del feature list.

---

# 63. Forma deseada de colaborar con Codex

Codex debería utilizar este documento como restricción de producto, no como orden de implementar todo.

Al recibir una tarea concreta:

1. inspeccionar el estado real del código;
2. entender la implementación existente;
3. identificar qué principio del documento es relevante;
4. modificar el mínimo necesario;
5. preservar comportamientos no relacionados;
6. evitar expandir scope;
7. explicar si una restricción técnica del NDS obliga a un compromiso;
8. preferir cambios reversibles cuando la UX todavía está en exploración.

No reescribir arquitectura por estética si no existe un problema práctico.

---

# 64. Cómo debería destilarse este documento a `AJUSTES.md`

La versión corta que acompañe permanentemente al repositorio debería contener únicamente lo que ayuda a Codex a tomar decisiones durante desarrollo.

Una estructura posible:

## Product intent

5–10 líneas sobre abrir+tocar, oído, juguete serio y no DAW.

## Hard design principles

Lista corta de invariantes.

## Current interaction model

Qué hacen actualmente teclado, pistas, frases y reproducción según el código real.

## Audio/timing principles

Especialmente:

- no depender de timing de REC;
- preservar silencio musical;
- no cuantización rígida.

## UI principles

- inferior = instrumento;
- superior = estado;
- botones físicos para estructura;
- pocas modalidades.

## Representation

0–11 / intervalos relativos.

## Scope discipline

No implementar backlog incidental.

## Known open problems

Solo los problemas que sigan abiertos tras inspeccionar el repositorio.

**Importante:** Codex debería construir ese `AJUSTES.md` comparando este texto con la implementación real. Si este documento describe una idea antigua que ya fue sustituida en código por una solución mejor y coherente con los principios, debe documentar el estado real, no resucitar automáticamente la idea antigua.

---

# 65. Resumen condensado del ADN

Si hubiera que reducir todo el proyecto a unas pocas frases:

> Esta DSi debe sentirse como un pequeño instrumento físico, no como un DAW portátil.

> La interacción principal es tocar de oído, recordar frases y combinarlas, no programar eventos.

> La aplicación debe eliminar fricción administrativa —especialmente la producida por grabación, sincronización y memoria de trabajo— sin corregir artificialmente la interpretación musical.

> El usuario debe poder empezar por la idea que tenga, incluida una melodía aislada, sin que la infraestructura del looper le imponga un orden de composición.

> Tres pistas, doce posiciones relativas, pocos botones y dos pantallas son una restricción deliberada: la calidad vendrá de hacer que esas pocas cosas funcionen extraordinariamente bien.

> Cuando haya que elegir entre más potencia y mejor feeling, normalmente elegir mejor feeling.

---

# 66. Nota final para Codex

Este documento contiene deliberadamente más historia y razonamiento del que debería permanecer en la documentación operativa del repositorio.

No tratar cada párrafo como requisito.

Buscar las **invariantes detrás de las decisiones**.

Una implementación concreta puede cambiar.

El objetivo es conservar el carácter del instrumento.

La pregunta final ante cualquier cambio debería ser:

> **¿Después de esto sigo sintiendo que cojo una DSi y toco, o empiezo a sentir que estoy manejando software musical?**

La primera respuesta es la dirección correcta.

# AGENTS.md — DS Stylophone / Looper

Reglas vigentes y mapa operativo del proyecto. No es un diario de sesiones, una cronología de bugs ni un manual de usuario.

## Fuente de verdad y mantenimiento

- `STATUS.md` resume el estado del producto; `KNOWN_ISSUES.md`, solo limitaciones o bugs abiertos.
- `design_chat/ADN_diseno_instrumento_DSi.md` conserva el razonamiento y la historia de producto. Sirve para recuperar intención, no como especificación literal: ante contradicciones, mandan las decisiones posteriores, el estado real del código y `STATUS.md`.
- `.codex-journal/` es historial, no instrucciones actuales.
- `KANBAN.md` contiene el backlog; `KANBAN.html` es su interfaz local. Si se modifica el HTML, exportar Markdown, actualizar `KANBAN.md` y revisar el diff. El agente no puede leer `localStorage`.
- Antes de implementar una feature, mover o crear su tarjeta `K-NNN` en `En curso` (máximo dos), con alcance y criterio de terminado. Al acabar, moverla a `Revisión`; solo pasa a `Hecho` con validación y documentación completas.
- Los bugs abiertos van en `KNOWN_ISSUES.md`, no en el tablero. Al resolver uno, actualizar `STATUS.md` y eliminarlo de `KNOWN_ISSUES.md` en el mismo cambio.
- Actualizar `MANUAL.md` junto con cualquier cambio visible de controles, interfaz, timbres, grabación, reproducción o diario.
- Mantener este archivo pequeño y estable: aquí solo van decisiones reutilizables; los detalles puntuales quedan en el script o en el historial.

## Visión y criterio de producto

- La DSi debe sentirse como un instrumento físico pequeño, inmediato, juguetón y personal; no como un DAW portátil ni una demo técnica. La aspiración es `POWER -> instrumento`.
- El núcleo es tocar de oído: buscar una melodía, descubrir intervalos, repetirla, recordarla, improvisar y combinar frases. Producir canciones terminadas es secundario.
- La relación ideal es `oigo -> busco -> toco -> repito -> recuerdo -> combino`. Una función vale especialmente si elimina fricción en esa cadena.
- Instrumento primero, editor después. Cuando haya que elegir entre más potencia y mejor feeling, preferir normalmente el feeling, la inmediatez y la coherencia.
- Debe funcionar en sesiones de 30 segundos sin preparación: no exigir por defecto proyecto, BPM, compás, escala, duración, pista base ni menús previos.
- La limitación es parte del carácter. Tres pistas, doce posiciones relativas, pocos botones y dos pantallas deben hacerse extraordinariamente bien antes de ampliar el sistema.
- El acabado se mide por respuesta, claridad, robustez, sonido agradable y ausencia de estados que obliguen a pensar en el software; no por el número de features.

## Principios musicales y de interacción

- La representación pública principal es relativa `0..11` y favorece pensar en saltos e intervalos. No mostrar nombres absolutos de notas ni convertir el producto en un curso formal de teoría salvo petición explícita.
- El instrumento ayuda a aprender de oído como andamio: memoria visual, recall y escucha; no resuelve la canción por el usuario ni sustituye la práctica.
- La pantalla inferior pertenece ante todo al teclado: targets cómodos, respuesta inmediata, geografía estable y poco ruido. La superior comunica estado y estructura musical, especialmente lo necesario para anticipar sincronización.
- Stylus = tocar; botones físicos = acciones discretas y estructurales. Mantener significados consistentes y feedback audiovisual inequívoco para tocar, armar, grabar, aceptar, cancelar, mutear, borrar y cambiar octava.
- Evitar interfaces organizadas alrededor de modos. Los estados breves inevitables deben corresponder a acciones naturales, ser visibles y devolver pronto al usuario a tocar.
- Una frase es algo que se toca, escucha y recuerda. El diario es memoria externa temporal, no un secuenciador; la reproducción debe servir tanto para oír la composición como para recordar material reciente.
- Conservar el silencio musical, pero no convertir automáticamente en música la latencia de mover el dedo, soltar el stylus o reaccionar a `REC`. Resolver límites de ciclo no equivale a cuantizar notas.
- No cuantizar la interpretación por defecto. Preferir preescucha, escucha y correcciones pequeñas a rejillas, edición microscópica o parámetros técnicos.
- No imponer un orden de composición: debe poder empezarse por una melodía aislada sin crear antes una base. Cualquier referencia temporal es infraestructura, no jerarquía creativa.
- Elegir pocas voces con carácter por su utilidad real al tocar: altura clara, ataque y sustain musicales, poca fatiga, convivencia entre capas y buen resultado en los altavoces de DSi. La variedad técnica no es un objetivo por sí sola.
- Cambio de octava sí; transposición global, overdub, secuenciador avanzado y persistencia compleja no son prioridades salvo que resuelvan una fricción observada.

## Como decidir con el usuario

- El usuario tiene criterio auditivo y una visión fuerte, pero no presupone conocimientos de teoría musical. No confundir falta de vocabulario técnico con falta de gusto o intención musical.
- Codex debe actuar también como guía musical: explicar cada decisión en lenguaje cotidiano y traducirla a consecuencias audibles y de interpretación. Introducir el término musical correcto después del concepto, no antes.
- Si existe una duda que cambie materialmente el feeling, el flujo, el sonido, la representación musical o el orden creativo, preguntar antes de fijarla. No delegar la decisión con un ambiguo "qué prefieres": presentar 2-3 alternativas concretas, sus consecuencias al tocar y una recomendación razonada.
- Separar explícitamente cuando sea útil: hecho musical, restricción de NDS, inferencia de UX y recomendación. No presentar una convención habitual de otros instrumentos como necesidad universal.
- Para decisiones que el usuario aún no pueda imaginar, proponer una prueba A/B pequeña y reversible en DSi, con una frase o escenario concreto que pueda tocar y comparar de oído.
- La experiencia real del usuario en hardware decide sobre una solución elegante en abstracto. Ante feedback como "no cae bien", "me pierdo" o "no apetece tocarlo", investigar la fricción antes de defender el modelo.
- No preguntar por detalles internos que puedan descubrirse en el repositorio ni por elecciones técnicas reversibles sin efecto perceptible. En esos casos, inspeccionar, elegir la opción mínima coherente y explicar el compromiso.

## Disciplina de alcance de producto

- Antes de proponer una feature, formular la fricción: `Cuando intento X, ocurre Y, me obliga a Z y rompe el flow`. "Otros loopers lo tienen" no es una justificación.
- Priorizar fricciones frecuentes y calidad de vida sobre amplitud funcional. Resolver el problema más pequeño que produzca el feeling correcto.
- Preferir mejorar una superficie o control existente a crear otra pantalla, modo, concepto o menú. Cada concepto público nuevo consume un presupuesto de complejidad limitado.
- No convertir ideas atractivas en compromiso de implementación. Capturarlas en el backlog y mantener pequeño y autocontenido el cambio actual.
- No rediseñar la interfaz ni la arquitectura desde cero si el estado actual ya resuelve bien el problema. Los cambios exploratorios de UX deben ser reversibles.
- Pregunta final para cualquier cambio visible: `Después de esto, sigo cogiendo una DSi y tocando, o empiezo a manejar software musical?`

## Herramientas y artefactos canónicos

| Utilidad | Uso | Regla |
| --- | --- | --- |
| BlocksDS | Compilación de la ROM | Es el único toolchain canónico. |
| `ds-stylophone.nds` | ROM final | Es la única ROM que reciben PC, emulador, FTP y consola. |
| `.tools/blocksds-test` | Build aislado de comprobación | No genera una ROM de distribución alternativa. |
| NO$GBA Debugger | CPU, memoria, TTY y capturas nativas | Usar `.tools/nogba-debug/NO$GBA.EXE`; complementa, no sustituye, melonDS. |
| melonDS | Compatibilidad, regresión visual y audio | Emulador oficial de compatibilidad. |
| `artifacts/` | Capturas y salidas de trabajo | Está ignorado por Git; versionar imágenes del manual solo si se trasladan deliberadamente a documentación. |

No usar Android, ADB ni DeSmuME en el ciclo normal.

## Política de evidencia en emulador

- No abrir ni ejecutar un emulador como comprobación genérica de que “algo funciona”. Toda ejecución debe producir una evidencia que se vaya a recoger y usar para validar una hipótesis o un criterio de terminado concreto.
- Usar NO$GBA únicamente cuando la evidencia pertinente sea TTY/trazas, estado de CPU o memoria, breakpoints, o una captura nativa necesaria para compatibilidad. Recoger la traza o inspección solicitada; arrancar la ROM sin leer esa salida no valida nada.
- Usar melonDS únicamente para una regresión UX con entradas y estado visual esperado explícitos, una captura que se vaya a revisar, o una prueba auditiva intencional. Un PNG o un hash distinto solo prueba que cambió la imagen: no prueba corrección por sí solo.
- No ejecutar melonDS ni generar capturas para cambios sin señal visual relevante —por ejemplo, una mejora de audio—. En ese caso usar la prueba de lógica adecuada y, si procede, una comprobación auditiva explícita; no declarar audio físico validado sin consola.
- Los pasos de un escenario deben corresponder al comportamiento modificado. No reutilizar un recorrido no relacionado ni declarar un bug resuelto porque el emulador no se cerró o porque una imagen cambió.

## Flujos de desarrollo y depuración

### Lógica y build

```powershell
python tests/test_loop_logic.py
g++ -std=c++17 -Iinclude tests/test_loop_logic.cpp -o tests/test_loop_logic.exe
.\tests\test_loop_logic.exe
.\build.ps1
.\tools\run-looper-flow.ps1
```

El flujo integrado debe terminar con `LOOPER_FLOW_RESULT=PASS`. El audio está desactivado por defecto; para una prueba auditiva explícita usar `-EnableSound`.

### NO$GBA: trazas y depuración

```powershell
.\tools\validate-nogba-smoke.ps1 -ManualTty
.\tools\validate-nogba-blocksds-real.ps1
```

- Cerrar NO$GBA antes de recompilar: puede bloquear `.map` y otros artefactos.
- Pasar la ruta de la ROM entre comillas, porque el workspace contiene espacios.
- Activar `Options > Debugger Setup > Debug I/O at 4FFFAxx > Enabled (TTY Messages etc.)`.
- Para ROMs BlocksDS, activar `Options > Debug eXception Setup > Disable all Warnings`: los avisos de I/O y DTCM (`0x02FF4000..0x02FF7FFF`) son falsos positivos conocidos.
- TTY es human-in-the-loop: abrir `Window > TTY Debug Messages`, pulsar `Copy` y permitir que el script lea el portapapeles. No depender de `DEBUGMSG.TXT` ni de automatización de GUI.
- El smoke exige `NOGBA_SMOKE_RESULT=PASS` con `SMOKE|STDERR|boot` y `SMOKE|NOCASH|boot`. La ROM real exige `NOGBA_BLOCKSDS_REAL_RESULT=PASS` y los marcadores `DSDBG|BOOT|blocksds|frame` y `DSDBG|NOCASH|boot`.
- NO$GBA no valida audio. No introducir notas durante estas pruebas; silenciarlo si hace falta garantizar silencio.

### NO$GBA: capturas visuales nativas

Para documentación o una inspección gráfica reproducible:

```powershell
.\tools\capture-nogba-screen.ps1 -Launch -Build -CloseLaunched -OutputPath .\artifacts\manual\inicio.png
```

El script construye la ROM canónica, abre una instancia limpia, captura la pantalla superior y táctil en PNG sin marcos (`256x384`), comprueba las dimensiones y devuelve `NOGBA_CAPTURE_RESULT=PASS` con SHA-256.

Para capturar un estado creado manualmente (recorte, ayuda, una frase, etc.), dejarlo visible y ejecutar:

```powershell
.\tools\capture-nogba-screen.ps1 -OutputPath .\artifacts\manual\estado.png
```

La ventana Emulator debe estar restaurada, en pantallas apiladas a escala `1x` y sin solapamientos. La herramienta toma los píxeles mostrados por Windows, no `PrintWindow`, evitando capturas negras. Adjuntar el PNG al chat permite inspeccionar UI, alineación, colores, texto y zonas táctiles. Más detalle en `docs/nogba-captures.md`.

Para recorrer checkpoints UX repetibles, usar un JSON de escenario:

```powershell
.\tools\run-nogba-ux-scenario.ps1 -ScenarioPath .\tools\nogba-ux-scenarios\mi-escenario.json -Build
```

El runner automatiza arranque, toques, esperas y capturas; los pasos `manual` permiten realizar botones DS y confirmar el estado real antes de capturarlo. La referencia es `tools/nogba-ux-scenarios/TEMPLATE.json` y el resultado esperado es `NOGBA_UX_SCENARIO_RESULT=PASS`.

Para recorridos UX totalmente automáticos usar melonDS, cuyo mapeo de teclado se controla temporalmente y se restaura al terminar:

```powershell
.\tools\run-melonds-ux-scenario.ps1 -ScenarioPath .\tools\melonds-ux-scenarios\record-track.json -Build
```

Admite botones, toques, esperas y capturas. Usarlo solo con un escenario cuyas entradas y estado visual esperado estén relacionados con el cambio; revisar las capturas, no solo sus hashes. NO$GBA mantiene el papel de TTY, CPU/memoria y compatibilidad.

### Distribución

```powershell
.\tools\upload-rom-ftp.ps1
```

El script recompila con `build.ps1` y sube exactamente `ds-stylophone.nds` a `/roms/nds/`. No mantener un segundo build devkitPro.

## Invariantes técnicas

- Las 12 zonas táctiles son cromáticas, iguales y usan exactamente índices `0`–`11`; nunca mostrar nombres de notas.
- Índice táctil y dibujo: `key = (x * 12) / 256`, con `x` calibrada en `0..255`.
- Probar todos los tonos, especialmente `E4=329 Hz`, `F4=349 Hz` y el límite `B4 < C5`.
- Dibujar en framebuffer de RAM y copiar a VRAM; no redibujar la bitmap completa directamente en VRAM cada frame. Las etiquetas de las 12 notas deben existir en el framebuffer.
- Las notas discretas usan `soundPlayPSG`; reservar `soundSetFreq` para vibrato. El PSG recibe `8 * Hz`; conservar los Hz musicales para interfaz y comprobaciones.
- Al diagnosticar audio, mostrar temporalmente nota, MIDI y Hz para separar mapeo, reproducción y mezcla con loops.

## Criterio de entrega

- Separar validación de lógica, de emulador y de audio físico.
- No declarar validado el audio físico ni una DSi XL sin evidencia explícita de esas pruebas.
- Si se resuelve un bloqueo, documentar únicamente la regla reutilizable o la corrección en el script correspondiente; no añadir una cronología aquí.

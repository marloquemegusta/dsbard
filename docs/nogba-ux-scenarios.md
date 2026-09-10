# Escenarios de captura UX con NO$GBA

`tools/run-nogba-ux-scenario.ps1` ejecuta una secuencia declarada en JSON en el monitor 2 y guarda capturas PNG nativas solo de cada checkpoint DS. Es la capa de flujo sobre `capture-nogba-screen.ps1`.

Partir de `tools/nogba-ux-scenarios/TEMPLATE.json`, darle un `name` único y ejecutarlo así:

```powershell
.\tools\run-nogba-ux-scenario.ps1 -ScenarioPath .\tools\nogba-ux-scenarios\mi-escenario.json -Build
```

Las imágenes se escriben en `artifacts/nogba/<name>/`. Además, cada ejecución produce `tty.txt` y `manifest.json`; el resultado esperado es `NOGBA_UX_SCENARIO_RESULT=PASS`.
Cerrar NO$GBA antes de ejecutarlo: el runner exige una instancia limpia para no tocar ni cerrar una sesión de depuración ajena.

Por defecto usa entrada `strict`: envía mensajes directamente a la ventana Debugger sin activar el foco, mover el cursor ni usar el teclado global. En la instalación actual, NO$GBA no consume esos mensajes de teclado; el runner lo detecta mediante `DSDBG|INPUT|...` y deja el escenario en FAIL. El stylus strict tampoco debe considerarse validado solo porque `PostMessage` devuelva `true`.

`-InputMode global` existe como diagnóstico: enfoca brevemente NO$GBA, usa `SendInput`/ratón global y restaura foco y cursor. El manifiesto marca `input_mode=global`; no es el camino recomendado para el uso normal porque puede interferir momentáneamente con el escritorio.

`-InputMode vjoy` es el modo recomendado para botones: usa el dispositivo vJoy configurado como Device 1 y envía A/B/SELECT/START/L/R/X/Y y el POV sin tomar el teclado ni el ratón. Los toques siguen usando el ratón global durante unos milisegundos y restauran foco y cursor; el manifiesto lo deja explícito como `touch_mode=global-restored`.

## Acciones disponibles

| Acción | Campos | Efecto |
| --- | --- | --- |
| `wait` | `ms` | Espera el tiempo indicado. |
| `touch` | `x`, `y`, `hold_ms` opcional | Toca coordenadas DS nativas; `x=0..255`, `y=192..383`. |
| `button` | `name`, `hold_ms` opcional | Envía un botón DS (`A`, `B`, `X`, `Y`, `L`, `R`, `START`, `SELECT`, `UP`, `DOWN`, `LEFT`, `RIGHT`) sin foco. |
| `tty` | `name` opcional | Exporta `Window > TTY Debug Messages > Copy` a `tty.txt`; nunca crea una captura de la ventana TTY. |
| `manual` | `instructions` | Detiene el flujo para realizar botones DS o un gesto complejo y confirma con Intro. |
| `capture` | `name` | Guarda `<name>.png` con el capturador nativo y sus comprobaciones. |

Los botones y toques strict son automáticos y repetibles. El checkpoint evita que el runner declare como válido un estado al que no llegó. Esto permite capturar recorte, silencios, ayuda, preview o cualquier estado de UX real sin inventar una ruta de entrada. Los pasos `manual` se conservan únicamente para gestos que todavía no estén expresados en el contrato.

Durante un checkpoint manual, no minimizar ni tapar la ventana Emulator y mantener la vista de pantallas apiladas a escala `1x`. Una vez completado el gesto, volver al terminal y pulsar Intro; el runner toma inmediatamente la captura solicitada.

## Smoke de entrada sin foco

```powershell
.\tools\run-nogba-ux-scenario.ps1 -ScenarioPath .\tools\nogba-ux-scenarios\background-input-smoke.json -InputMode vjoy -Build
```

La prueba deja `00-inicial.png`, `01-tras-boton-SELECT.png`, `02-tras-touch.png`, `tty.txt` y `manifest.json`. El PNG solo sirve para revisar el estado visible del checkpoint; el TTY se valida como texto contra los marcadores declarados. `tools/nogba-ux-scenarios/key-probe.json` sirve para diagnosticar mapas de teclado, pero no convierte el teclado sintetizado en una ruta soportada.

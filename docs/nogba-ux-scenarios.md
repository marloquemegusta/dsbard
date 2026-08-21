# Escenarios de captura UX con NO$GBA

`tools/run-nogba-ux-scenario.ps1` ejecuta una secuencia declarada en JSON y guarda capturas PNG nativas de cada checkpoint. Es la capa de flujo sobre `capture-nogba-screen.ps1`.

Partir de `tools/nogba-ux-scenarios/TEMPLATE.json`, darle un `name` único y ejecutarlo así:

```powershell
.\tools\run-nogba-ux-scenario.ps1 -ScenarioPath .\tools\nogba-ux-scenarios\mi-escenario.json -Build
```

Las imágenes se escriben en `artifacts/nogba/<name>/` y el resultado esperado es `NOGBA_UX_SCENARIO_RESULT=PASS`.
Cerrar NO$GBA antes de ejecutarlo: el runner exige una instancia limpia para no tocar ni cerrar una sesión de depuración ajena.

## Acciones disponibles

| Acción | Campos | Efecto |
| --- | --- | --- |
| `wait` | `ms` | Espera el tiempo indicado. |
| `touch` | `x`, `y`, `hold_ms` opcional | Toca coordenadas DS nativas; `x=0..255`, `y=192..383`. |
| `manual` | `instructions` | Detiene el flujo para realizar botones DS o un gesto complejo y confirma con Intro. |
| `capture` | `name` | Guarda `<name>.png` con el capturador nativo y sus comprobaciones. |

Los toques son automáticos y repetibles. Los pasos `manual` son intencionados: NO$GBA recibe botones por un mapeo de teclado o mando local y no ofrece una API fiable para inyectarlos. El checkpoint evita que el runner declare como válido un estado al que no llegó. Esto permite capturar recorte, silencios, ayuda, preview o cualquier estado de UX real sin inventar una ruta de entrada.

Durante un checkpoint manual, no minimizar ni tapar la ventana Emulator y mantener la vista de pantallas apiladas a escala `1x`. Una vez completado el gesto, volver al terminal y pulsar Intro; el runner toma inmediatamente la captura solicitada.

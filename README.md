# DS Stylophone / Looper

Instrumento homebrew para Nintendo DS/DSi: 12 franjas cromáticas táctiles y tres loops independientes de eventos musicales. No guarda datos entre sesiones.

## Compilar

Con devkitPro/devkitARM instalado:

```sh
make
```

También se puede compilar con la imagen oficial `devkitpro/devkitarm:latest` usando el script `build.ps1`.

Las pruebas de la lógica de loops se ejecutan con `g++ -std=c++17 -Iinclude tests/test_loop_logic.cpp -o tests/test_loop_logic.exe; tests/test_loop_logic.exe`.

Si melonDS está descargado en `.tools/melonds`, `.\run-emulator.ps1` abre la ROM directamente.

## Controles

- Touchscreen: tocar y arrastrar notas C–B.
- D-pad arriba/abajo: octava 2–6.
- D-pad izquierda/derecha: seleccionar Track 1–3.
- A: grabar/detener grabación de la pista seleccionada.
- B corto: deshacer la última regrabación; B mantenido: limpiar de forma deliberada.
- START: pausar/reanudar el transporte.
- SELECT: mute/unmute de la pista.
- X: cambiar timbre; Y sustain mientras se mantiene; L vibrato; R glide entre notas.

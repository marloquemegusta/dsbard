# Estado

- Funciona: interfaz directa, 12 zonas táctiles, octavas, tres tracks estáticos, grabación/reproducción de eventos, mute y pausa con fase, undo/reemplazo, sustain, vibrato y glide básicos.
- Probado: tests host-side C++ y estructurales; build DS reproducible en la imagen oficial `devkitpro/devkitarm:latest`; ROM cargada en melonDS 1.1; captura visual confirma ambas pantallas y el arrastre del stylus sobre una franja.
- Queda: verificación auditiva formal y validación física en DSi XL; el entorno no permite afirmar esas dos últimas como completadas.
- Build exacto: `docker run --rm -v "${PWD}:/source" -w /source devkitpro/devkitarm:latest make`
- ROM: `ds-stylophone.nds` (177152 bytes en la última build).

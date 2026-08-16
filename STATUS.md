# Estado

- Funciona: interfaz directa, 12 zonas táctiles, octavas, tres tracks estáticos, grabación/reproducción de eventos, mute y pausa con fase, undo/reemplazo, sustain, vibrato y glide básicos.
- Probado: tests host-side C++ y estructurales; build DS reproducible en la imagen oficial `devkitpro/devkitarm:latest`; ROM lanzada en melonDS 1.1 durante una prueba de proceso.
- Queda: verificación visual/audio interactiva en emulador y validación física en DSi XL; el entorno no permite afirmar esas dos últimas como completadas.
- Build exacto: `docker run --rm -v "${PWD}:/source" -w /source devkitpro/devkitarm:latest make`
- ROM: `ds-stylophone.nds`.

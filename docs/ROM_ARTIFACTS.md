# ROMs de referencia y reconstrucción

Release de referencia: [reconstruction-baseline-2026-09-10](https://github.com/marloquemegusta/dsbard/releases/tag/reconstruction-baseline-2026-09-10).

## ROM buena conservada en la Nintendo DS

- Archivo local: `recovered/ds-stylophone-from-ds.nds`
- Asset de release: `ds-stylophone-from-ds.nds`
- Procedencia: copia extraída de la ROM que estaba instalada en la DS y que se utilizó como referencia funcional y visual.
- SHA-256: `e826ecde2da3a1413d874ae39772d1d6389f7311f1ebc95a79f447d6b14d30b7`

Esta ROM es la referencia conocida como buena. No representa código fuente recuperado y no debe sobrescribirse.

## ROM reconstruida actual

- Archivo local: `ds-stylophone.nds`
- Asset de release: `ds-stylophone.nds`
- Procedencia: build del estado actual de `main`, reconstruido a partir de la copia antigua, la ROM de la DS, las conversaciones y los artefactos recuperados.
- SHA-256: `7235d5b6c32ce794f386144fa1e296122ee575522fe1df2572a7740666db669d`

Esta ROM es un punto intermedio del esfuerzo de reconstrucción. No se debe asumir que sea idéntica a la ROM buena hasta completar la comparación funcional, visual y auditiva en hardware.

## Otros artefactos locales

`recovered/ftp-verify.nds` es una copia de verificación de una transferencia FTP anterior. Se conserva localmente, pero no forma parte de la pareja principal de ROMs publicada.

## Verificación

El archivo `SHA256SUMS.txt` de la release permite comprobar que las descargas no se han alterado.

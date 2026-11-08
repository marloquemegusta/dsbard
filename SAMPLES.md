# Samples

Los 52 samples son síntesis offline propia (13 instrumentos PCM × 4 raíces MIDI), generada por el proyecto; no contienen grabaciones ni material de terceros y se pueden redistribuir con el proyecto como CC0-equivalente.

- Formato: PCM lineal mono, 8-bit signed, 22.050 Hz, binario raw para `soundPlaySample`.
- Multisampling: raíces MIDI 48, 60, 72 y 84 para cada instrumento; se escoge la raíz más cercana y se reproduce con la frecuencia corregida.
- Instrumentos PCM (13): `RHODES`, `KALIMBA`, `BASS`, `PLUCK`, `GLASS`, `PAD`, `HEROIC`, `HORN`, `ANIME`, `CINEMA`, `STRINGS`, `E GUITAR` y `FLUTE`. La ROM también conserva cuatro referencias PSG sin samples: `SQUARE`, `PULSE`, `THIN` y `DARK` (17 timbres en total).
- Puntos de loop PCM: `PAD` (35.280 muestras), `HEROIC` / `HORN` / `ANIME` / `CINEMA` / `FLUTE` (14.332 muestras), `STRINGS` (17.640 muestras) y `E GUITAR` (11.025 muestras); los demás samples (`RHODES`, `KALIMBA`, `BASS`, `PLUCK`, `GLASS`) son one-shot con envolvente completa y no repiten.
- Volumen nominal: 82/127 para loops y 100/127 para live; las notas por debajo de MIDI 72 reciben compensación progresiva hasta el límite 127 para contrarrestar la pérdida de graves de los altavoces DS.

La técnica está documentada en la guía de audio de BlocksDS y en la API de sonido de libnds; Maxmod sigue siendo apropiado para música y SFX empaquetados, pero aquí se usa el mezclador hardware directo para poder elegir frecuencia por nota y mantener el presupuesto de voces pequeño.

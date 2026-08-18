# Samples

Los 24 samples son síntesis offline propia, generada por `tools/generate_samples.py`; no contienen grabaciones ni material de terceros y se pueden redistribuir con el proyecto como CC0-equivalente.

- Formato: PCM lineal mono, 8-bit signed, 22.050 Hz, binario raw para `soundPlaySample`.
- Multisampling: raíces MIDI 48, 60, 72 y 84 para cada instrumento; se escoge la raíz más cercana y se reproduce con la frecuencia corregida.
- Instrumentos PCM: RHODES (armónicos suaves), KALIMBA (pluck corto), BASS (fundamental redonda), PLUCK (armónicos medios), GLASS (parciales inarmónicos) y PAD (doble seno con ataque lento). La ROM también conserva cuatro referencias PSG sin samples: SQUARE, PULSE, THIN y DARK.
- El PAD usa un loop de reproducción en el punto 1,6 s; los demás samples tienen envolvente completa y no repiten.
- Volumen nominal: 82/127 para loops y 100/127 para live; las notas por debajo de MIDI 72 reciben compensación progresiva hasta el límite 127 para contrarrestar la pérdida de graves de los altavoces DS.

La técnica está documentada en la guía de audio de BlocksDS y en la API de sonido de libnds; Maxmod sigue siendo apropiado para música y SFX empaquetados, pero aquí se usa el mezclador hardware directo para poder elegir frecuencia por nota y mantener el presupuesto de voces pequeño.

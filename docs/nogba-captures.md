# Capturas reproducibles de NO$GBA

Este repositorio captura la **zona cliente nativa** de la ventana `NO$GBA Emulator`: un PNG de `256x384` píxeles, con la pantalla superior seguida de la táctil, sin bordes, título ni controles de Windows.

El comando reproducible para documentación es:

```powershell
.\tools\capture-nogba-screen.ps1 -Launch -Build -CloseLaunched -OutputPath .\artifacts\manual\inicio.png
```

El script construye la ROM canónica, abre una instancia limpia de NO$GBA, inicia la ejecución desde el Debugger, toma la captura y cierra solamente la instancia que él mismo abrió. Termina con `NOGBA_CAPTURE_RESULT=PASS`, la ruta, las dimensiones y el SHA-256 del PNG.

Para documentar un estado al que se ha llegado manualmente (una frase en recorte, ayuda, etc.), déjalo visible en NO$GBA y ejecuta:

```powershell
.\tools\capture-nogba-screen.ps1 -OutputPath .\artifacts\manual\recorte.png
```

La herramienta recorta los pequeños márgenes del cliente de NO$GBA y rechaza cualquier escala distinta de la vista nativa `1x`. Antes de capturar, el emulador debe estar restaurado, con las pantallas DS apiladas, y sin otra ventana encima: la captura se obtiene de los píxeles que realmente muestra Windows. Esta decisión evita PNG negros o incompletos que `PrintWindow` puede producir con NO$GBA.

Para revisión gráfica, adjunta el PNG generado a la conversación. La imagen conserva píxeles nativos, así que permite comprobar alineación, colores, texto, zonas táctiles y cortes de interfaz. Los PNG de trabajo van en `artifacts/` y no deben versionarse salvo que se elijan deliberadamente como imágenes del manual.

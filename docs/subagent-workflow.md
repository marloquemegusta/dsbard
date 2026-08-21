# Modo subagentes

> **Estado: DEPRECADO para este proyecto (2026-08-22).** El intento de usar este proceso para el rediseño del compositor produjo una implementación parcial, varias rondas de correcciones y una ROM que no expresaba el cambio de producto esperado. No activar este modo por defecto ni usarlo para retomar el rediseño. La especificación recuperable está en [`note-composer-redesign-brief.md`](note-composer-redesign-brief.md).

Este documento se conserva como registro de un proceso experimental, no como una instrucción operativa vigente.

Proceso general para trabajar con varias perspectivas de Codex sobre una feature, un bug o una revisión. Este documento no describe una feature concreta del producto: define cómo organizar el trabajo cuando el usuario diga «usa el modo subagentes», «modo equipo» o una instrucción equivalente.

## 1. Qué significa activar el modo

El modo subagentes no significa que todos los agentes trabajen a la vez ni que todos editen el repositorio. Significa que el trabajo se divide en roles con responsabilidades separadas y salidas verificables.

El agente principal actúa como coordinador:

- interpreta el objetivo del usuario;
- elige el tipo de equipo;
- entrega a cada subagente solo el contexto que necesita;
- integra sus resultados;
- controla Git y la rama de trabajo;
- decide cuándo una ronda está terminada.

Los subagentes no se comunican directamente entre sí. El coordinador pasa los artefactos relevantes de una fase a la siguiente. Así se evita que una suposición incorrecta se propague sin revisión.

## 2. Selección del equipo

No existe un equipo único para todas las tareas. El coordinador selecciona el equipo según la forma del problema.

### Feature nueva o cambio de comportamiento

Usar cuando el usuario pide añadir una capacidad o rediseñar un flujo.

```text
arqueólogo → proponente/implementador → usuario virtual → revisor → arquitecto si hay bloqueo
```

- **Arqueólogo:** localiza código reutilizable, dependencias, tests y límites actuales. Solo lectura.
- **Proponente/implementador:** propone la solución mínima y la programa en la rama actual. Es el único agente que edita durante la ronda.
- **Usuario virtual:** ejecuta el flujo con el contrato visible y el manual, sin inventar controles ni conocer las intenciones internas del diseño.
- **Revisor:** busca fricción, estados innecesarios, botones redundantes, errores y regresiones.
- **Arquitecto:** resuelve conflictos o decisiones estructurales difíciles. No se llama por defecto.

### Bug o comportamiento incorrecto

Usar cuando algo existente no funciona, regresa o produce un resultado inesperado.

```text
reproductor → investigador de causa → revisor de hipótesis → implementador → verificador
```

- **Reproductor:** convierte el informe en pasos reproducibles y determina qué ocurre realmente.
- **Investigador de causa:** sigue datos, llamadas, estados y condiciones hasta aislar la causa probable.
- **Revisor de hipótesis:** intenta falsar la explicación y comprueba que no se está corrigiendo solo el síntoma.
- **Implementador:** aplica el cambio mínimo y añade una prueba de regresión.
- **Verificador:** ejecuta la prueba específica, las pruebas cercanas y las comprobaciones requeridas por el proyecto.

Si el bug no es reproducible, no se implementa una corrección especulativa. Se mejora primero la evidencia: trazas, aserciones, logging o un caso mínimo.

### Revisión de código o de una rama

Usar cuando el usuario pide auditar cambios ya hechos.

```text
lector de diff → revisor de lógica → revisor de producto/UX si aplica → coordinador
```

Estos agentes no modifican código. Devuelven hallazgos con ubicación, severidad, impacto y corrección sugerida.

### Investigación técnica

Usar cuando todavía no está claro dónde está algo o si una solución es viable.

```text
investigadores independientes → coordinador → decisión o plan
```

Se pueden ejecutar en paralelo porque no dependen unos de otros. No se crea una implementación hasta consolidar sus resultados.

## 3. Modelos y razonamiento

Configuración por defecto:

- arqueólogo, reproductor, usuario virtual e implementador: `gpt-5.6-luna`, razonamiento medio;
- revisor: `gpt-5.6-luna`, razonamiento alto;
- arquitecto: `gpt-5.6-terra`, razonamiento alto, solo cuando exista un bloqueo o una decisión de arquitectura.

Terra no se usa como revisor normal para controlar coste. Luna alto basta para detectar problemas concretos cuando recibe un diff, un contrato y una traza bien definidos.

## 4. Rondas de trabajo

Una ronda siempre tiene un objetivo comprobable.

### Ronda de feature

1. El arqueólogo entrega el mapa de reutilización.
2. El proponente define el cambio mínimo y programa un candidato.
3. El candidato queda descrito mediante un contrato visible: controles, estados, pantallas y resultados.
4. El usuario virtual recorre el caso de uso paso a paso.
5. El revisor clasifica fallos como bloqueantes, importantes, cosméticos o falsos problemas.
6. El coordinador decide aceptar, corregir o descartar.
7. Si se acepta, la siguiente ronda parte de ese estado.

Una ronda no se considera terminada porque el código compile. Debe superar el flujo de uso que motivó el cambio.

### Ronda de bug

1. Se fija una reproducción mínima.
2. Se formula una hipótesis de causa.
3. Otro agente intenta refutarla.
4. Se implementa la corrección mínima.
5. Se añade una regresión automatizada cuando sea posible.
6. Se verifica el caso original y los caminos cercanos.

## 5. Simulación del usuario

Cuando no sea práctico ejecutar todavía el hardware o el emulador, el usuario virtual trabaja como un emulador semántico, no como una opinión abstracta.

Entrada:

- controles disponibles;
- estado inicial;
- manual o contrato visible;
- objetivo del usuario;
- límites del escenario.

Salida:

```text
Estado visible:
Acción:
Resultado esperado:
Resultado observado según el código:
Duda o fricción:
Recuperación posible:
```

El usuario virtual no puede asumir que una zona táctil tiene una función no documentada, ni resolver mentalmente una transición ambigua. Si queda bloqueado, el diseño o el manual han fallado.

La simulación no sustituye una prueba visual, de compatibilidad o de audio. Solo decide si la arquitectura y sus transiciones merecen llegar a esa prueba.

## 6. Git y seguimiento del código

Se usa una única rama exploratoria secuencial. No se crean worktrees ni ramas paralelas por defecto.

```text
v1.4
└── codex/exploratory-note-editor
    ├── explore(round-01): propuesta inicial
    ├── explore(round-01): corregir bloqueo de selección
    ├── explore(round-01): añadir regresión
    ├── explore(round-02): simplificar flujo
    └── ...
```

Reglas:

- cada turno de implementación hace un commit;
- los commits intermedios pueden estar rotos;
- no se reescribe ni se aplasta la historia durante la exploración;
- las rondas son secuenciales y parten del último estado aceptado;
- los agentes de análisis y revisión no editan el código;
- el coordinador decide qué commits se consideran un checkpoint aceptado;
- se pueden usar tags ligeros como `exploration-r1-accepted`;
- al terminar, se revisa la secuencia completa antes de decidir qué integrar o limpiar.

La rama conserva tanto las soluciones aceptadas como los intentos descartados. La historia es parte del experimento.

## 7. Paralelización

La ruta principal es secuencial cuando una fase depende de la anterior:

```text
propuesta → implementación → simulación → revisión → decisión
```

Solo se paralelizan tareas independientes, por ejemplo:

- inventario de código y recopilación de tests existentes;
- dos investigaciones distintas sobre el origen de un bug;
- usuario virtual y verificador técnico leyendo el mismo candidato;
- revisión de lógica y revisión de documentación.

No se paralelizan dos agentes editando los mismos archivos ni dos rondas de diseño que dependan entre sí. Eso aumenta la integración y no mejora la calidad.

## 8. Contratos de salida

Cada subagente debe devolver una salida breve y estructurada.

### Arqueólogo

```text
Reutilizar:
Adaptar:
Eliminar o aislar:
Archivos y símbolos relevantes:
Riesgos:
```

### Implementador

```text
Arquitectura aplicada:
Archivos modificados:
Commit:
Tests ejecutados:
Limitaciones conocidas:
```

### Usuario virtual

```text
Pasos ejecutados:
Bloqueos:
Ambigüedades:
Acciones innecesarias:
Recuperación de errores:
```

### Revisor

```text
Bloqueantes:
Importantes:
Cosméticos:
Simplificaciones propuestas:
Veredicto: aceptar | corregir | descartar
```

### Arquitecto

```text
Decisión:
Motivo:
Cambio mínimo recomendado:
Consecuencia aceptada:
```

## 9. Criterios generales de calidad

Una propuesta es buena cuando:

- reutiliza más de lo que reemplaza;
- reduce o mantiene la cantidad de conceptos públicos;
- tiene un flujo visible y recuperable;
- permite probar la hipótesis con un caso concreto;
- añade una regresión cuando corrige un bug;
- no necesita que el usuario conozca la implementación;
- no introduce flexibilidad que el escenario no necesita;
- separa validación de lógica, interfaz, emulador y hardware real.

La pregunta final para una feature visible es:

> Después de este cambio, ¿el usuario está haciendo la tarea o está gestionando el software?

## 10. Cómo invocarlo

El usuario puede decir:

```text
Usa el modo subagentes para diseñar e implementar [feature].
```

o:

```text
Usa el modo subagentes para investigar y corregir [bug].
```

El coordinador elegirá automáticamente el equipo apropiado, explicará la primera ronda y mantendrá la rama exploratoria y los commits según este documento.

Si el usuario quiere cambiar el nivel de paralelización, el modelo de un rol o el número de rondas, puede indicarlo explícitamente. En caso contrario, se aplican estas reglas por defecto.

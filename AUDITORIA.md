# Auditoría — DIAMOND I · PAPIELGUAPO

Versión 1.0.1. Auditoría técnica realizada sobre el código y el plugin compilado; no es una certificación independiente de terceros.

## Interfaz y apertura

La ventana se crea siempre a **626 × 290 px, 50%**, incluso tras cerrar una ventana ampliada o cargar un estado antiguo que guardaba un tamaño grande. Los parámetros de audio siguen guardándose y automatizándose. El tamaño deja de formar parte del estado del plugin.

Se conserva el escalado proporcional de Diamond II. Arrastre de la esquina y menú contextual sobre el fondo con 25%, 33%, 50%, 60%, 75%, 100%, 125%, 150% y 200%. Límites: 313 × 145 a 2504 × 1160 px. El menú usa una referencia débil al editor para evitar acceder a un editor destruido cuando termine una acción asíncrona.

El verificador comprueba tamaño inicial, nueve escalas, límites, reapertura y restauración de un estado antiguo. Genera capturas del editor real a seis tamaños, incluida la apertura al 50%.

## Revisión del procesamiento

* H2–H13 son contribuciones Chebyshev independientes con taper cuadrático y ganancia 1/n; no comparten un denominador de normalización.
* Los 17 parámetros usan APVTS y se conservan los identificadores de automatización de la versión anterior.
* Procesamiento mono/estéreo, suavizado de 25 ms y tres ramas FIR 2x/4x/8x preasignadas, con compensación a 99 muestras y cambio de modo por transición.
* El callback usa buffers preasignados, atomics y DSP por muestra; no abre archivos, crea filtros ni toma locks. El contador de `new/new[]` verifica un caso de bloques mayores al máximo anunciado. No cubre todas las API de asignación del sistema.
* Se sanean entradas/salidas no finitas y se evita el crecimiento exponencial de los polinomios fuera de ±1 mediante prolongación lineal. No hay limitador de salida.
* Estado XML validado por etiqueta y valores finitos. Los estados antiguos con `editorWidth` son compatibles y ese atributo ya no determina el tamaño de apertura.

## Evidencias ejecutadas

El informe [docs/RESULTADOS.md](docs/RESULTADOS.md) recoge las mediciones. Las salidas originales de las herramientas, con rutas locales normalizadas para publicación, están en `docs/verification.txt` y `docs/pluginval-10.txt`.

La suite incluye 108 combinaciones espectrales: H2–H13 con senoide de 100 Hz, 44,1/48/96 kHz y 2x/4x/8x. Comprueba frecuencia FFT, ganancia y armónicos residuales. También mide transparencia, aliasing de tonos altos, estado, aislamiento estéreo, NaN/Inf, cambios de frecuencia de muestreo, bypass, latencia, suma de generadores y rendimiento.

La validación de compatibilidad solicitada, **pluginval nivel 10**, ha terminado con **SUCCESS**, semilla 4281, sobre el VST3 1.0.1. Incluye las pruebas Editor, Open editor whilst processing, Editor Automation y Fuzz parameters; no se han desactivado las pruebas GUI. La suite propia termina con **FAILURES 0**. No se debe confundir pluginval con el validador oficial de Steinberg, que requiere un ejecutable adicional.

La instalación en `C:/Program Files/Common Files/VST3` se ha completado con elevación UAC y su hash coincide con el binario validado; véase `docs/installation.json`. La inspección visual de la captura a 626 px confirma la apertura pequeña. Los resultados espectrales siguen mostrando 108/108 combinaciones correctas y un peor armónico residual de −84,769 dBc en las condiciones descritas.

## Límites conocidos

1. La separación espectral está calibrada con senoide de pico 1 a la entrada del generador. A otros niveles aparecen órdenes inferiores de la misma paridad; con música hay intermodulación.
2. El modo 2x puede producir aliasing elevado con órdenes altos y señales agudas. Las pruebas de tonos altos y sus resultados completos están en `docs/aliasing.csv`.
3. Las tres ramas funcionan continuamente: el cambio de modo no reserva memoria, pero aumenta el consumo respecto a una única rama.
4. Una cola de continua puede persistir brevemente después de bajar los faders a cero. La transparencia se comprueba en condiciones estables.
5. Combinar armónicos y ganancias puede superar 0 dBFS; el ajuste de salida es manual.
6. Construcción ejecutada con Visual Studio 2026. La configuración VS2022 se incluye, pero no se ha ejecutado con ese compilador.
7. No se ha realizado todavía una prueba de uso dentro de FL Studio. Las pruebas de herramientas y capturas no sustituyen la revisión en el DAW.

## Procedencia

Se inspeccionaron los proyectos locales originales Diamond II y D2F sin modificarlos. Se adaptó el panel visual de Diamond II, preservando su aviso MIT. D2F utiliza x²/x³ con normalización compartida; por eso se eligió la alternativa Chebyshev autorizada. Este plugin no se presenta como una copia sonora exacta de D2F. Los hashes de los archivos inspeccionados figuran en `docs/reference-hashes.json`.

El código se distribuye bajo AGPL-3.0-or-later, con JUCE como dependencia externa. No se publican carpetas de compilación, credenciales ni archivos de los proyectos originales completos.

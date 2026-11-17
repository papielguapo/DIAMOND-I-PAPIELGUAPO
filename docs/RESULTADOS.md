# Resultado de la verificación — 17/11/2026

**Compilación Release x64 ejecutada correctamente** con CMake 4.4, Visual Studio 2026 / MSVC 19.51, Windows SDK 10.0.26100 y JUCE 9.0.4 local. Producto: `DIAMOND I - HARMONIC EMULATION.vst3`, versión 1.0.1, fabricante PAPIELGUAPO. El proyecto permite seleccionar el generador VS2022; ese compilador no está instalado y no se ha probado aquí.

Se han ejecutado `DiamondVerify` y pluginval contra el procesador y el bundle compilados. No se ha abierto FL Studio. El plugin se ha instalado en `C:/Program Files/Common Files/VST3`, verificando que su SHA256 coincide con el binario validado.

| Comprobación | Resultado |
|---|---|
| Senoide 100 Hz, H2–H13 individuales, 3 frecuencias de muestreo × 3 modos OS | 108/108 combinaciones correctas |
| Frecuencia FFT | Componente esperado entre 200 y 1300 Hz, dentro de una resolución de bin |
| Ganancia de cada orden a 100% | Amplitud 1/n, tolerancia 0,5 dB superada |
| Mayor armónico residual H2–H20, excluido el orden seleccionado | Peor caso −84,769 dBc |
| Todos los faders a cero y ganancias unidad | Error máximo inferior a 1e−7 respecto a la entrada retardada |
| Estado | 17 parámetros restaurados; datos inválidos rechazados; tamaño pequeño al reabrir |
| Bloques de 1025 muestras con máximo anunciado 128 | Procesados sin asignaciones C++ `new/new[]` detectadas |
| Estéreo | Canal silencioso sin excitación ni fuga |
| Protección | Entradas NaN/Inf contenidas; salida finita |
| Repreparación | Historial reiniciado correctamente a 44,1/48/96 kHz |
| Suma de generadores | Coincide con suma de contribuciones independientes; sin denominador compartido |
| Bypass | Coincide con entrada retardada, error <1e−7 tras estabilizar |
| Rendimiento medido | 0,321694 s de tiempo de pared por 1 s de audio estéreo a 96 kHz, 12 generadores activos |
| Latencia | 99 muestras en todos los modos OS |
| Interfaz | Capturas a 313, 626, 751, 1252, 1878 y 2504 px; apertura fija 626 × 290 |
| pluginval | Nivel 10, `SUCCESS`, incluidas Editor, Open editor whilst processing, Editor Automation y Fuzz parameters |

El tiempo de proceso depende del equipo y de la carga concurrente; no es una garantía para cualquier ordenador o tamaño de bloque. No se ha ejecutado el validador oficial de Steinberg: pluginval indica que esa prueba se omite por no disponer de su ejecutable configurado. Las ventanas se revisan mediante capturas del editor real y las pruebas GUI de pluginval nivel 10.

## Aliasing medido

Se mide mediante demodulación coherente el componente H13 plegado a la banda base después de un segundo de estabilización. No es una medición de toda la energía de aliasing ni un THD+N global. Los datos completos están en `aliasing.csv`.

| Sample rate | Entrada | Componente plegado | 2x | 4x | 8x |
|---|---|---|---|---|---|
| 44,1 kHz | 7 kHz | 2,8 kHz | −22,28 dBFS | −136,63 dBFS | −137,16 dBFS |
| 48 kHz | 7 kHz | 5 kHz | −22,28 dBFS | −141,61 dBFS | −142,61 dBFS |
| 96 kHz | 11 kHz | 47 kHz | −33,71 dBFS | −133,40 dBFS | −133,79 dBFS |

2x no dispone de ancho de banda interno suficiente para H13 en estas pruebas. Se ofrece como modo económico y se documenta esa limitación. Los modos 4x/8x superan el umbral comprobado de −75 dBFS para esos componentes. Las cifras cercanas al suelo numérico no equivalen a un rechazo garantizado para toda señal.

La separación de órdenes se calibra a pico 1, 0 dBFS, en la entrada del generador. A otros niveles aparecen órdenes inferiores de la misma paridad. Con música se genera intermodulación. El motor es la alternativa Chebyshev autorizada, no una reproducción exacta de D2F.

## Evidencias

* `verification.txt`: salida completa del verificador; `FAILURES 0`.
* `spectral.csv`: resultados de las 108 combinaciones espectrales.
* `aliasing.csv`: nueve mediciones de tonos altos.
* `pluginval-10.txt`: registro del bundle final, nivel 10; `SUCCESS`.
* `interface-*.png`: capturas del editor real.
* `reference-hashes.json`: identificación de los archivos de referencia inspeccionados (rutas de referencia normalizadas).

Los originales Diamond II y D2F no se han modificado. La instalación en la carpeta VST3 compartida está completada y documentada en `installation.json`. La comprobación dentro de FL Studio queda pendiente en el host del usuario.

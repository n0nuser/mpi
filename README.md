# Práctica MPI

## Integrantes

* [Enrique Hernández Hernández](https://github.com/Enriranjan)
* [Francisco Javier Gallego Lahera](https://github.com/fjvgallego)
* [Sergio García González](https://github.com/AnOrdinaryUsser)
* Pablo Jesús González Rubio

## Nota Final: 9

## Número perfecto

Un número perfecto es un entero positivo que es igual a la suma de sus divisores propios positivos. 
Así, 6 es un número perfecto porque sus divisores propios son 1, 2 y 3; y 6 = 1 + 2 + 3.
Los siguientes números perfectos son 28, 496 y 8128.

```
28 = 1 + 2 + 4 + 7 + 14
496 = 1 + 2 + 4 + 8 + 16 + 31 + 62 + 124 + 248
8128 = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 127 + 254 + 508 + 1016 + 2032 + 4064
```

Se desea realizar una aplicación que permita determinar si un número introducido es perfecto, y si no lo es, si es excesivo o defectivo.
**No se trata de optimizar algoritmos de localización de números perfectos**, se van a calcular los divisores positivos uno a uno y se van a ir sumando.

## Se pide

**Realizar un programa en MPI que permita distribuir el cálculo en N procesos**. Para ello habrá: 
  - **Un proceso encargado de la E/S que**:
    - Será inicialmente el conocedor del número a comprobar siendo que el resto de procesos no lo c conocen inicialmente (puede ser un número de más de 10 cifras) y puede *introducirse por teclado o línea de entrada*.
    - A partir de ese número deberá repartirse la tarea entre los procesos.
    - *Irá recibiendo mensajes y acumulando la suma y el número de los divisores obtenidos por cada proceso*.
    - Además, deberá de recibir de cada proceso un mensaje indicando que ha terminado, la suma de los divisores descubiertos por el proceso, así como el tiempo de cálculo que ha empleado. La suma la comparará con la suma que ha ido acumulando.
    - El último proceso mandará al proceso 0 un mensaje de finalización con la suma total (recogida del resto de procesos). El proceso 0 comprobará la suma total que ha calculado con la enviada por el último proceso.
  - **Nº procesos calculadores que**:
    - Esperarán a que el proceso de E/S les mande la tarea.
    - Irán dividiendo y cada vez que descubran un divisor se lo mandarán al proceso 0 y lo acumularán.
    - Una vez terminada su tarea indicarán al proceso 0 que han terminado y le proporcionarán la suma parcial y el tiempo que han empleado.
    - También mandarán un mensaje al proceso siguiente (si es el x al x+1), si existe, con la suma de los divisores obtenidos por él.
    - Si un proceso x recibe un mensaje del proceso x-1 se lo sumará al valor de la suma de los divisores obtenido por él y se lo mandará al proceso x+1.
    - Si el proceso x es último proceso le mandará el mensaje al proceso 0, que corresponderá con la suma de todos los divisores.
**Una vez realizado el programa realizar un estudio de rendimiento en el cual se vea que**:
  - Aumentando el número de procesos se reduce el tiempo de cálculo, o no, hasta un cierto momento (por ej.: 1 (secuencial), 2, 4, 8, 16, 32,...)
**Identificar problemas y proponer mejoras en el reparto de tareas a partir de los resultados obtenidos**.

## Fecha de entrega 9 de Abril de 2021

# Laboratorio 6
4 de Septiembre de 2024
## Respuestas Ejercicio 2.
### ¿ Cual fue la diferencia entre la impresion del primer programa y el segundo ?
En el programa de la parte B las impresiones no necesariamente están ordenadas, al ejecutar el programa varias veces se ven diferencias en el orden.  Mientras tanto, en el programa de la parte C las impresiones siempre se muestran en orden.
### ¿ A qué se debió el comportamiento descrito en la respuesta anterior ?
En el programa de la parte B se crean todos los hilos y el trabajo se realiza de forma paralela, luego se unen los hilos conforme terminan su trabajo, haciendo que el resultado sea desordenado porque cualquier hilo puede unirse antes o después.
En el programa de la parte C, después de crear cada hilo, se une, es por esto que siempre se logra observar que las impresiones están en orden, ya que los hilos se unen en orden.
## Video Explicando Codigo
[Link](https://youtu.be/TnBJLkai1w0)
## Comandos de Compilacion y Ejecucion
### Ejercicio 1
```
gcc -o bin/Ej1 src/Ej1.cpp -lpthread -lstdc++
```
```
./bin/Ej1
```
### Ejercicio 2
```
gcc -o bin/Ej2 src/Ej2.cpp -lpthread -lstdc++
```
```
./bin/Ej2
```
### Ejercicio 3
```
gcc -o bin/Ej3 src/Ej3.cpp -lpthread -lstdc++
```
```
./bin/Ej3
```

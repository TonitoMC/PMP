# PMP
12 de Octubre de 2024 | José Antonio Mérida Castejón | Pablo Daniel Barillas Moreno
### NOTA EJERCICIO 3
No se si era intencional el funcionamiento de los consumidores pero para que consumieran 4 patas, 1 asiento y 1 respaldo al final se quedan con piezas 'en su inventario'. Todo sale en el reporte y los productores dejan de producir, pero como es random la pieza que sale puede que los consumidores paren con algunas patas de mas jaladas del buffer. La otra solucion era que solo agarraran las piezas que necesitaran pero ahi puede quedarse atorado el programa si ya no necesitan mas patas y el buffer se llena de patas :)

## Video Explicatorio y Documentos
Los documentos se encuentran en el PDF Micros_lab08.pdf con las respuestas teóricas y explicaciones

Link al video

### Ejercicio 2
Compilación

```
gcc -o bin/Ejercicio2 src/Ejercicio2.cpp -lpthread -lstdc++
```

Ejecución

```
./bin/Ejercicio2
```

### Ejercicio 3
Compilación

```
gcc -o bin/Ejercicio3BModificado src/Ejercicio3BModificado.cpp -lpthread -lstdc++
```

Ejecución

```
./bin/Ejercicio3BModificado
```
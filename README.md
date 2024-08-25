# Proyecto #2
## Ubicación de los Archivos
- **src:** El código fuente escrito en C para una versión paralela y una versión secuencial del programa
- **bin:** Los ejecutables resultantes de la compilación de los archivos
- PDF del informe del proyecto en el directorio principal del repositorio
## Compilación y Corrida
### Versión Paralela:
Compilar a /bin
```
gcc -fopenmp -o bin/PSOParallel src/PSOParallel.c
```
Correr de /bin
```
./bin/PSOParallel
```
### Versión Secuencial:
Compilar a /bin
```
gcc -fopenmp -o bin/PSOSequential src/PSOSequential.c
```
Correr de /bin
```
./bin/PSOSequential
```
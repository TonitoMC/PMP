# Proyecto #2
## Ubicación de los Archivos
- **src:** El código fuente escrito en C para una versión paralela y una versión secuencial del programa
- **bin:** Los ejecutables resultantes de la compilación de los archivos
- PDF del informe del proyecto en el directorio principal del repositorio
## Compilación y Corrida
### Versión Secuencial Síncrona:
Compilar a /bin
```
gcc -fopenmp -o bin/PSOSeqS src/PSOSeqS.c
```
Correr de /bin
```
./bin/PSOSeqS
```
### Versión Paralela Síncrona:
Compilar a /bin
```
gcc -fopenmp -o bin/PSOParS src/PSOParS.c
```
Correr de /bin
```
./bin/PSOParS
```
### Versión Secuencial Asíncrona:
Compilar a /bin
```
gcc -fopenmp -o bin/PSOSeqA src/PSOSeqA.c
```
Correr de /bin
```
./bin/PSOSeqA
```
### Versión Paralela Asíncrona:
Compilar a /bin
```
gcc -fopenmp -o bin/PSOParA src/PSOParA.c
```
Correr de /bin
```
./bin/PSOParA
```
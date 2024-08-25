/*
-------------------------------------------------------------
PSOSequential.c
-------------------------------------------------------------
UNIVERSIDAD DEL VALLE DE GUATEMALA
CC3086 - Programacion de Microprocesadores
-------------------------------------------------------------
Algoritmo de optimización por enjambre de partículas, ejecutado
de manera secuencial.
-------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define W 0.5
#define C1 1.5
#define C2 1.5


// Función que buscamos optimizar
long double f(long double x, long double y){
    return x + y;
}

// Struct para organizar las coordenadas
struct Coords{
    long double x, y;
};

// Struct para manejar las partículas
struct Particle{
    long double vx, vy, bestfitness;
    struct Coords currentCoords, bestCoords;
};

// Evaluación de fitness (aptitud) de una partícula
long double fitness(struct Particle *p){
    return f(p->currentCoords.x, p->currentCoords.y);
};

// Actualiza la posición de una particula y compara si existe un 
long double update(struct Particle *p, struct Coords globalBestCoords){
    // Crea variables aleatorias entre 0 y 1 para la actualización
    long double r1 = (long double) rand() / RAND_MAX;
    long double r2 = (long double) rand() / RAND_MAX;
     
    // Actualizamos la velocidad en cada eje conforme las formulas
    p->vx = W * C1 * r1 * (p->bestCoords.x - p->currentCoords.x) + C2 * r2 * (globalBestCoords.x - p->currentCoords.x);
    p->vy = W * C1 * r1 * (p->bestCoords.y - p->currentCoords.y) + C2 * r2 * (globalBestCoords.y - p->currentCoords.y);

    // Sumamos la velocidad a las posiciones en cada eje para movernos a un nuevo punto
    p->currentCoords.x += p->vx;
    p->currentCoords.y += p->vy;

    

}

int main() {
    printf("Hola");

    return 0;
}
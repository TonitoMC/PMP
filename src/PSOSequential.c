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
#include <math.h>

#define W 0.25
#define C1 0.5
#define C2 0.5


// Función que buscamos optimizar
double f(double x, double y){
    return (x - 2) * (x - 2) + (y - 3) * (y - 3);
}

// Struct para organizar las coordenadas
struct Coords{
    double x, y;
};

// Struct para manejar las partículas
struct Particle{
    double vx, vy, bestFitness;
    struct Coords currentCoords, bestCoords;
};

// Evaluación de fitness (aptitud) de una partícula
double fitness(struct Particle *p){
    return f(p->currentCoords.x, p->currentCoords.y);
};

// Actualiza la posición de una particula y compara si existe un 
double update(struct Particle *p, struct Coords *globalBestCoords, double *globalBestFitness){
    // Crea variables aleatorias entre 0 y 1 para la actualización
    double r1 = (double) rand() / RAND_MAX;
    double r2 = (double) rand() / RAND_MAX;
     
    // Actualizamos la velocidad en cada eje conforme las formulas
    p->vx = W * p->vx + C1 * r1 * (p->bestCoords.x - p->currentCoords.x) + C2 * r2 * (globalBestCoords->x - p->currentCoords.x);
    p->vy = W * p->vy + C1 * r1 * (p->bestCoords.y - p->currentCoords.y) + C2 * r2 * (globalBestCoords->y - p->currentCoords.y);

    // Sumamos la velocidad a las posiciones en cada eje para movernos a un nuevo punto
    p->currentCoords.x += p->vx;
    p->currentCoords.y += p->vy;

    double currentFitness = fitness(p);
    if (currentFitness < p->bestFitness){
        p->bestFitness = currentFitness;
        p->bestCoords = p->currentCoords;
        if (currentFitness < *globalBestFitness){
            *globalBestFitness = currentFitness;
            *globalBestCoords = p->currentCoords;
        }
    }
}


int main() {
    double start = omp_get_wtime();
    struct Particle particles[40];
    struct Coords globalBestCoords;

    globalBestCoords.x = 0.0;
    globalBestCoords.y = 0.0;

    double globalBestFitness = INFINITY;

    for (int i = 0; i < 40; i++){

        particles[i].vx = (double) rand() / RAND_MAX * 20 - 10;
        particles[i].vy = (double) rand() / RAND_MAX * 20 - 10;

        particles[i].currentCoords.x = (double) rand() / RAND_MAX * 20 - 10;
        particles[i].currentCoords.y = (double) rand() / RAND_MAX * 20 - 10;

        particles[i].bestFitness = INFINITY;

        particles[i].bestCoords.x = 0;
        particles[i].bestCoords.y = 0;
    }
    for (int i = 0; i < 40; i++){
        printf("Particula: %d, Vx: %f, Vy: %f\n", i, particles[i].vx, particles[i].vy);
    }
    for (int i = 0; i < 100; i++){
        for (int i = 0; i < 40; i++){
        update(&particles[i], &globalBestCoords, &globalBestFitness);
        }
    }
    double end = omp_get_wtime();
    printf("%.15f, %.15f, %f", globalBestCoords.x, globalBestCoords.y, end - start);
    return 0;
    }
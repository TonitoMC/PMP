/*
-------------------------------------------------------------
PSOSeqS.c
-------------------------------------------------------------
UNIVERSIDAD DEL VALLE DE GUATEMALA
CC3086 - Programacion de Microprocesadores
-------------------------------------------------------------
Algoritmo de optimización por enjambre de partículas síncrono
ejecutado de manera secuencial.
-------------------------------------------------------------
*/
#define _USE_MATH_DEFINES
#define _CRT_RAND_S
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

// Parámetros constantes de la simulación
#define W 0.5               // Inercia, tendencia de la partícula a seguir en movimiento
#define C1 1.5              // Factor personal
#define C2 1.5              // Factor social
#define NUM_ITERS 100       // Numero de iteraciones
#define NUM_PARTICLES 1000  // Numero de Particulas

// Calcula el valor de la función que deseamos minimzar (función Ackley)
double f(double x, double y) {
    double term1 = -20.0 * exp(-0.2 * sqrt(0.5 * (x * x + y * y)));
    double term2 = -exp(0.5 * (cos(2.0 * M_PI * x) + cos(2.0 * M_PI * y)));
    return term1 + term2 + 20.0 + M_E;
}

// Estructura para representar coordenadas con precisión Double
struct Coords {
    double x, y;
};

// Estructura para representar una partícula
struct Particle {
    double vx, vy, bestFitness; // Velocidades y Mejor Fitness (Menor valor de la función evaluada)
    struct Coords currentCoords, bestCoords; // Mejores coordenadas y coordenadas en las que se encuentra
};

// Evaluación del fitness correspondiente a las coordenadas actuales de una partícula p
double fitness(struct Particle *p) {
    return f(p->currentCoords.x, p->currentCoords.y);
}

// Actualiza la posición de una particula y compara si existe un 
double update(struct Particle *p, struct Coords *globalBestCoords, double *globalBestFitness, struct Coords *iterationBestCoords, double *iterationBestFitness){
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
        if (currentFitness < *iterationBestFitness){
            *iterationBestFitness = currentFitness;
            *iterationBestCoords = p->currentCoords;
        }
    }
}


int main() {
    double start = omp_get_wtime();
    struct Particle particles[NUM_PARTICLES];
    struct Coords globalBestCoords;

    globalBestCoords.x = 0.0;
    globalBestCoords.y = 0.0;

    double globalBestFitness = INFINITY;

    for (int i = 0; i < NUM_PARTICLES; i++){

        particles[i].vx = (double) rand() / RAND_MAX * 64 - 32;
        particles[i].vy = (double) rand() / RAND_MAX * 64 - 32;

        particles[i].currentCoords.x = (double) rand() / RAND_MAX * 64 - 32;
        particles[i].currentCoords.y = (double) rand() / RAND_MAX * 64 - 32;

        particles[i].bestFitness = INFINITY;

        particles[i].bestCoords.x = 0;
        particles[i].bestCoords.y = 0;
    }
    for (int i = 0; i < NUM_ITERS; i++){
        double iterationBestFitness = globalBestFitness;
        struct Coords iterationBestCoords = globalBestCoords;
        for (int j = 0; j < NUM_PARTICLES; j++){
            update(&particles[j], &globalBestCoords, &globalBestFitness, &iterationBestCoords, &iterationBestFitness);
        }
        if (iterationBestFitness < globalBestFitness){
            globalBestCoords = iterationBestCoords;
            globalBestFitness = iterationBestFitness;
            printf("Nuevo Mejor Fitness: %.15f, encontrado por hilo: %d. Iter: %d\n", globalBestFitness, omp_get_thread_num(), i);
        }
    }
    double totalDistance = 0.0;
    for(int i = 0; i < NUM_PARTICLES; i++){
        double distanceFromBest = sqrt(pow(globalBestCoords.x - particles[i].currentCoords.x, 2) 
                                + pow(globalBestCoords.y - particles[i].currentCoords.y, 2));
        totalDistance += distanceFromBest;
    }
    double end = omp_get_wtime();
    printf("Mejores Coordenadas: (%.15f, %.15f),Tiempo de Corrida %f, Distancia Promedio: %.15f", globalBestCoords.x, globalBestCoords.y, end - start, totalDistance / NUM_PARTICLES);
    return 0;
    }
/*
-------------------------------------------------------------
PSOSeqA.c
-------------------------------------------------------------
UNIVERSIDAD DEL VALLE DE GUATEMALA
CC3086 - Programacion de Microprocesadores
-------------------------------------------------------------
Algoritmo de optimización por enjambre de partículas asíncrono
ejecutado de manera secuencial.
-------------------------------------------------------------
*/
#define _USE_MATH_DEFINES
#define _CRT_RAND_S // Ensure rand_s is available
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

// Parámetros constantes de la simulación
#define W 0.5   // Inercia, tendencia de la partícula a seguir en movimiento
#define C1 1.5  // Factor personal
#define C2 1.5  // Factor social

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

// Función de actualización de una partícula, se corre una vez por iteración
void update(struct Particle *p, struct Coords *globalBestCoords, double *globalBestFitness) {
    unsigned int r1, r2;
    rand_s(&r1); // Generate a random number
    rand_s(&r2); // Generate another random number

    // Convert to double in the range [0, 1]
    double dr1 = (double) r1 / UINT_MAX;
    double dr2 = (double) r2 / UINT_MAX;

    // Actualización de la velocidad en X para la partícula según la fórmula
    p->vx = W * p->vx + C1 * dr1 * (p->bestCoords.x - p->currentCoords.x) + C2 * dr2 * (globalBestCoords->x - p->currentCoords.x);
    // Actualización de las velocidad en Y para la partícula según la fórmula
    p->vy = W * p->vy + C1 * dr1 * (p->bestCoords.y - p->currentCoords.y) + C2 * dr2 * (globalBestCoords->y - p->currentCoords.y);

    // Se actualiza la posición de la partícula en ambos ejes
    p->currentCoords.x += p->vx;
    p->currentCoords.y += p->vy;

    // Cálculo de fitness de la función
    double currentFitness = fitness(p);
    // Se actualiza en caso que se haya encontrado una mejor posición
    if (currentFitness < p->bestFitness){
        p->bestFitness = currentFitness;
        p->bestCoords = p->currentCoords;
        // Se compara con la mejor posición global en caso que deba ser actualizada
        if (currentFitness < *globalBestFitness){
            *globalBestFitness = currentFitness;
            *globalBestCoords = p->currentCoords;
            printf("Nuevo mejor Fitness: %f, encontrado por hilo: %d\n", currentFitness, omp_get_thread_num());
        }
    }
}

int main() {
    double start = omp_get_wtime();
    struct Particle particles[1000];
    struct Coords globalBestCoords;

    globalBestCoords.x = 0.0;
    globalBestCoords.y = 0.0;

    double globalBestFitness = INFINITY;

    for (int i = 0; i < 1000; i++){
        unsigned int r1, r2, r3, r4;
        rand_s(&r1);
        rand_s(&r2);
        rand_s(&r3);
        rand_s(&r4);

        particles[i].vx = (double) r1 / UINT_MAX * 64 - 32;
        particles[i].vy = (double) r2 / UINT_MAX * 64 - 32;

        particles[i].currentCoords.x = (double) r3 / UINT_MAX * 64 - 32;
        particles[i].currentCoords.y = (double) r4 / UINT_MAX * 64 - 32;

        particles[i].bestFitness = INFINITY;

        particles[i].bestCoords.x = 0;
        particles[i].bestCoords.y = 0;
    }

    for (int i = 0; i < 100; i++){
        for (int j = 0; j < 1000; j++){
            update(&particles[j], &globalBestCoords, &globalBestFitness);
        }
    }

    double totalDistance = 0.0;
    for(int i = 0; i < 1000; i++){
        double distanceFromBest = sqrt(pow(globalBestCoords.x - particles[i].currentCoords.x, 2) 
                                + pow(globalBestCoords.y - particles[i].currentCoords.y, 2));
        totalDistance += distanceFromBest;
    }

    double end = omp_get_wtime();
    printf("%.15f, %.15f, %f, Distancia Promedio: %.15f", globalBestCoords.x, globalBestCoords.y, end - start, totalDistance / 1000);
    return 0;
}
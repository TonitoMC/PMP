/*
-------------------------------------------------------------
PSOParA.c
-------------------------------------------------------------
UNIVERSIDAD DEL VALLE DE GUATEMALA
CC3086 - Programacion de Microprocesadores
-------------------------------------------------------------
Algoritmo de optimización por enjambre de partículas asíncrono
ejecutado en paralelo utilizando la librería OpenMP. 
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

// Función de actualización de una partícula, se corre una vez por iteración
void update(struct Particle *p, struct Coords *globalBestCoords, double *globalBestFitness) {
    // Generacion de variables aleatorias
    unsigned int r1, r2;
    rand_s(&r1);
    rand_s(&r2);
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
        // Se compara con la mejor posición global antes de entrar a critical, para evitar saturación
        if (currentFitness < *globalBestFitness){
            // Sección critica para aseugrarnos que solamente un hilo pueda modificar la variable a la vez
            #pragma omp critical
            {
                // De encontrar una mejor coordenada se actualizan las variables globales
                if (currentFitness < *globalBestFitness){
                    *globalBestFitness = currentFitness;
                    *globalBestCoords = p->currentCoords;
                    printf("Nuevo mejor Fitness: %.15f, encontrado por hilo: %d\n", currentFitness, omp_get_thread_num());
                }
            }
        }
    }
}

int main() {
    // Inicialización de Variables
    double start = omp_get_wtime();     // Tiempo de Inicio
    struct Particle particles[NUM_PARTICLES];    // Array dónde se almacenan las partículas
    struct Coords globalBestCoords;     // Las mejores coordenadas

    // Inicializamos en 0, este puede ser cualquier valor ya que independientemente
    // se actualizará en la primera partícula que se evalúe
    globalBestCoords.x = 0.0;
    globalBestCoords.y = 0.0;

    // Ya que buscamos minimizar, el fitness inicial es infinito para que se actualice
    // en la primera partícula evaluada
    double globalBestFitness = INFINITY;

    // Sección paralela para la inicialización de partículas
    #pragma omp parallel for
    for (int i = 0; i < NUM_PARTICLES; i++) {
        unsigned int r1, r2, r3, r4, r5, r6;
        rand_s(&r1);
        rand_s(&r2);
        rand_s(&r3);
        rand_s(&r4);
        rand_s(&r5);
        rand_s(&r6);

        // Velocidades iniciales
        particles[i].vx = (double) r1 / UINT_MAX * 64 - 32;
        particles[i].vy = (double) r2 / UINT_MAX * 64 - 32;

        // Posiciones iniciales en el intervalo [-32,32]
        particles[i].currentCoords.x = (double) r3 / UINT_MAX * 64 - 32;
        particles[i].currentCoords.y = (double) r4 / UINT_MAX * 64 - 32;

        // Fitness inicial en infinito, para que se actualice en la primera iteración
        particles[i].bestFitness = INFINITY;

        // Inicialización de coordenadas, estos valores se actualizarán en la primera iteración
        particles[i].bestCoords.x = 0;
        particles[i].bestCoords.y = 0;
    }

    // Creamos una nueva región paralela
    #pragma omp parallel
    {
        // Llevando a cabo el número de iteraciones
        for (int i = 0; i < NUM_ITERS; i++) {
            // Designamos un 'subenjambre' a cada hilo y continuamos las iteraciones
            // sin realizar alguna espera a sincronización. Las variables globales
            // se actualizan cada vez que se encuentre un mejor valor
            #pragma omp for nowait
            for (int j = 0; j < NUM_PARTICLES; j++) {
                update(&particles[j], &globalBestCoords, &globalBestFitness);
            }
        }
    }
    // Calculo de la distancia promedio de todas las partículas respecto a la mejor posición encontrada
    double totalDistance = 0;
    #pragma omp parallel for reduction(+:totalDistance)
    for (int i = 0; i < NUM_PARTICLES; i++) {
        double distanceFromBest = sqrt(pow(globalBestCoords.x - particles[i].currentCoords.x, 2) 
                                    + pow(globalBestCoords.y - particles[i].currentCoords.y, 2));
        totalDistance += distanceFromBest;
    }
    // Impresion de resultados finales
    double end = omp_get_wtime();
    printf("Mejores Coordenadas: (%.15f, %.15f),Tiempo de Corrida %f, Distancia Promedio: %.15f", globalBestCoords.x, globalBestCoords.y, end - start, totalDistance / NUM_PARTICLES);
    return 0;
}
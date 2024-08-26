#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define W 0.5
#define C1 1.5
#define C2 1.5

double f(double x, double y) {
    double term1 = -20.0 * exp(-0.2 * sqrt(0.5 * (x * x + y * y)));
    double term2 = -exp(0.5 * (cos(2.0 * M_PI * x) + cos(2.0 * M_PI * y)));
    return term1 + term2 + 20.0 + M_E;
}

struct Coords {
    double x, y;
};

struct Particle {
    double vx, vy, bestFitness;
    struct Coords currentCoords, bestCoords;
};

double fitness(struct Particle *p) {
    return f(p->currentCoords.x, p->currentCoords.y);
}

void update(struct Particle *p, struct Coords *globalBestCoords, double *globalBestFitness) {
    double r1 = (double) rand() / RAND_MAX;
    double r2 = (double) rand() / RAND_MAX;

    p->vx = W * p->vx + C1 * r1 * (p->bestCoords.x - p->currentCoords.x) + C2 * r2 * (globalBestCoords->x - p->currentCoords.x);
    p->vy = W * p->vy + C1 * r1 * (p->bestCoords.y - p->currentCoords.y) + C2 * r2 * (globalBestCoords->y - p->currentCoords.y);

    p->currentCoords.x += p->vx;
    p->currentCoords.y += p->vy;

    double currentFitness = fitness(p);
    if (currentFitness < p->bestFitness) {
        p->bestFitness = currentFitness;
        p->bestCoords = p->currentCoords;
    }
}

int main() {
    double start = omp_get_wtime();
    struct Particle particles[1000];
    struct Coords globalBestCoords;

    globalBestCoords.x = 0.0;
    globalBestCoords.y = 0.0;

    double globalBestFitness = INFINITY;

    #pragma omp parallel for
    for (int i = 0; i < 1000; i++) {
        particles[i].vx = (double) rand() / RAND_MAX * 64 - 32;
        particles[i].vy = (double) rand() / RAND_MAX * 64 - 32;

        particles[i].currentCoords.x = (double) rand() / RAND_MAX * 64 - 32;
        particles[i].currentCoords.y = (double) rand() / RAND_MAX * 64 - 32;

        particles[i].bestFitness = INFINITY;

        particles[i].bestCoords.x = 0;
        particles[i].bestCoords.y = 0;
    }
    #pragma omp parallel
    for (int i = 0; i < 1000; i++) {
        struct Coords iterationBestCoords = globalBestCoords;
        double iterationBestFitness = globalBestFitness;

        #pragma omp for
        for (int j = 0; j < 1000; j++) {
            update(&particles[j], &iterationBestCoords, &iterationBestFitness);
        }
        #pragma omp single
        {
            if (iterationBestFitness < globalBestFitness) {
                globalBestFitness = iterationBestFitness;
                globalBestCoords = iterationBestCoords;
            }
            //printf("Iteration %d: Best Coords: (%.15f, %.15f), Best Fitness: %f\n", i, globalBestCoords.x, globalBestCoords.y, globalBestFitness);
        }
    }

    double end = omp_get_wtime();
    printf("Final Best Coords: (%.15f, %.15f), Execution Time: %f\n", globalBestCoords.x, globalBestCoords.y, end - start);
    return 0;
}
#include <stdio.h>
#include <omp.h>

// Calcula la sumatoria hasta un numero (long para evitar overflow)
// de manera paralela utilizando un reduction
long long longSumParallel(long long n) {
    long long longSum = 0;
    #pragma omp parallel
    {
        #pragma omp for reduction(+:longSum)
        for (long long i = 0; i <= n; i++) {
            longSum += i;
        }
    }
    return longSum;
}

// Calcula la sumatoria hasta un numero de manera secuencial
long long longSumSequential(long long n) {
    long long longSum = 0;
    for (long long i = 0; i <= n; i++) {
        longSum += i;
    }
    return longSum;
}

int main() {
    // Corre la funcion de sumatoria en paralelo e imprime el tiempo de corrida
    double parallelStart = omp_get_wtime();
    long long parallelResult = longSumParallel(10000000);
    double parallelFinish = omp_get_wtime();
    printf("Paralelo: %lld, Tiempo de Corrida: %f\n", parallelResult, parallelFinish - parallelStart);

    // Corre la funcion de sumatoria secuencial e imprime el tiempo de corrida
    double sequentialStart = omp_get_wtime();
    long long sequentialResult = longSumSequential(10000000);
    double sequentialFinish = omp_get_wtime();
    printf("Secuencial: %lld, Tiempo de Corrida: %f\n", sequentialResult, sequentialFinish - sequentialStart);

    return 0;
}
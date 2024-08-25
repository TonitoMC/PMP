#include <stdio.h>
#include <omp.h>
//TODO comment
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

long long longSumSequential(long long n) {
    long long longSum = 0;
    for (long long i = 0; i <= n; i++) {
        longSum += i;
    }
    return longSum;
}

int main() {
    double parallelStart = omp_get_wtime();
    long long parallelResult = longSumParallel(10000000);
    double parallelFinish = omp_get_wtime();
    printf("Paralelo: %lld, Tiempo de Corrida: %f\n", parallelResult, parallelFinish - parallelStart);

    double sequentialStart = omp_get_wtime();
    long long sequentialResult = longSumSequential(10000000);
    double sequentialFinish = omp_get_wtime();
    printf("Secuencial: %lld, Tiempo de Corrida: %f\n", sequentialResult, sequentialFinish - sequentialStart);

    return 0;
}
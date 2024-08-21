#include <stdio.h>
#include <omp.h>

int factorial(int n){
    if (n == 0){
        return 1;
    } else{
        return n * factorial(n - 1);
    }
};

int fibonacci(int n){
    if(n <= 1){
        return n;
    } else{
        return fibonacci(n-1) + fibonacci(n-2);
    }
}

int sum_of_squares(int n){
    int sum = 0;
    for(int i = 1; i <= n; i++){
        sum += i * i;
    }
    return sum;
}

int main(){
    int n = 50;
    printf("Inicial: %d \n", n);
    double start_global = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                double start_time = omp_get_wtime();
                int fact = factorial(n);
                double end_time = omp_get_wtime();
                printf("Factorial: %d, calculado por hilo %d. Tiempo: %f\n", fact, omp_get_thread_num(), end_time - start_time);
            }
            #pragma omp section
            {
                double start_time = omp_get_wtime();
                int fib = fibonacci(n);
                double end_time = omp_get_wtime();
                printf("Fibonacci: %d, calculado por hilo %d. Tiempo: %f\n", fib, omp_get_thread_num(), end_time - start_time);
            }
            #pragma omp section
            {
                double start_time = omp_get_wtime();
                int sqr = sum_of_squares(n);
                double end_time = omp_get_wtime();
                printf("Suma de Cuadrados hasta N: %d, calculado por hilo %d. Tiempo: %f", sqr, omp_get_thread_num(), end_time - start_time);
            }
        }
    }
}
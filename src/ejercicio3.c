#include <stdio.h>
#include <omp.h>
#include <windows.h>
#define N 1000

// Funcion que calcula el n-vo numero de la secuencia Fibonacci
// de manera recursiva
int fibonacci(int n){
    if (n <= 1){
        return n;
    }
    return fibonacci(n-1) + fibonacci(n-2);
}

// Funcion que determina si un numero es primo comprobando
// el residuo por cualquier posible divisor.
int is_prime(int num) {
    if (num <= 1) return 0;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

// Funcion que comprueba si un numero es primo desde 0 hasta 
// limit
int count_primes(int limit) {
    int count = 0;
    for (int i = 2; i <= limit; i++) {
        if (is_prime(i)) {
            count++;
        }
    }
    return count;
}

// Funcion que busca estimar el valor de Pi por el metodo de
// Montecarlo. 
double monte_carlo_pi(int num_samples) {
    int inside_circle = 0;
    for (int i = 0; i < num_samples; i++) {
        // Genera valores aleatorios para las coordenadas entre 0 y 1
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;
        // Verifica si se encuentran dentro del cuarto de circulo
        if (x*x + y*y <= 1.0) {
            inside_circle++;
        }
    }
    // La proporcion de muestras dentro del circulo aproxima pi/4, encontramos
    // pi
    return (4.0 * inside_circle) / num_samples;
}

int main(){
    // Toma el tiempo inicial de la ejecucion del programa (Global)
    double start_global = omp_get_wtime();
    #pragma omp parallel
    {
        // Creamos la region de las secciones que queremos crear
        #pragma omp sections
        {
            // Primera seccion, calculo Fibonacci #40
            #pragma omp section
            {
                // Mide el tiempo de corrida del calculo e imprime resultados
                double start_time = omp_get_wtime();
                int fib = fibonacci(40);
                double end_time = omp_get_wtime();
                printf("Fibonnacci #50: %d, Thread: %d. Tiempo: %f\n", fib, omp_get_thread_num(), end_time - start_time);
            }
            // Segunda seccion, conteo de primos hasta 1,750,000
            #pragma omp section
            {
                // Mide el tiempo de corrida del calculo e imprime resultados
                double start_time = omp_get_wtime();
                int prime_count = count_primes(1750000);
                double end_time = omp_get_wtime();
                printf("Numeros Primos Menores a 1000000 %d, Thread: %d. Tiempo: %f\n", prime_count, omp_get_thread_num(), end_time - start_time);
            }
            // Tercera seccion, estimacion de pi con Montecarlo con 17,500,000 muestras
            #pragma omp section
            {
                // Mide el tiempo de corrida del calculo e imprime resultados
                double start_time = omp_get_wtime();
                double pi = monte_carlo_pi(17500000);
                double end_time = omp_get_wtime();
                printf("Pi Usando Montecarlo (1000000 muestras): %f, Thread: %d. Tiempo: %f\n", pi, omp_get_thread_num(), end_time - start_time);
            }
        }
        // Cada region lleva a cabo los calculos en un thread y se sincronizan implicitamente en este punto
    }
    // Obtenemos el tiempo de corrida global y lo imprimimos
    double end_global = omp_get_wtime();
    printf("Tiempo total: %f\n", end_global - start_global);
    return 0;
}
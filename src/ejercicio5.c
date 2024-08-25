#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 131072

int taskCount = 0;

// Funcion recursiva para contar las ocurrencias de 'key' en el arreglo 'a'
long count_key_recursive(long *a, long start, long end, long key, int depth, int max_depth) {
    int thread_id = omp_get_thread_num();

    // El punto en el que iniciamos a realizar los tasks de conteo, calculado en el Main
    // para que la cantidad de tasks sea igual a la cantidad de hilos
    if (depth >= max_depth) {
        long count = 0;
        // Cuenta las ocurrencias de 'key' en el rango actual
        for (long i = start; i <= end; i++) {
            if (a[i] == key) count++;
        }
        printf("Thread %d - Realizando task de conteo en el rango [%ld, %ld]: %ld\n", thread_id, start, end, count);
        taskCount++;
        return count;
    }

    // Calcula el punto medio del rango actual, crea un task para la izquierda y la derecha
    long mid = (start + end) / 2;
    long left_count = 0, right_count = 0;

    #pragma omp task shared(left_count)
    {
        left_count = count_key_recursive(a, start, mid, key, depth + 1, max_depth);
    }

    #pragma omp task shared(right_count)
    {
        right_count = count_key_recursive(a, mid + 1, end, key, depth + 1, max_depth);
    }

    // Espera a que ambas tareas terminen y combina los resultados al conteo
    #pragma omp taskwait
    printf("Thread %d - Combinando resultados para el rango: [%ld, %ld] : %ld\n", thread_id, start, end, left_count + right_count);
    return left_count + right_count;
}

int main() {
    // Establece el número de hilos a 16 por consistencia.
    omp_set_num_threads(16);

    // Inicializar el array
    long a[N], key = 42, nkey = 0;
    for (long i = 0; i < N; i++) a[i] = rand() % N;
    a[N % 43] = key; 
    a[N % 73] = key; 
    a[N % 3] = key;

    int max_threads = omp_get_max_threads();

    // Calcular la profundidad máxima para la creación de tareas paralelas
    // basado en la cantidad de hilos disponibles y la profundidad a la que
    // se deben crear las tareas para tener el numero exacto de tareas e hilos.
    int max_depth = 0;
    while ((1 << max_depth) < max_threads) max_depth++;

    // Region paralela
    #pragma omp parallel
    {
        // Single para la llamada inicial
        #pragma omp single
        {
            nkey = count_key_recursive(a, 0, N - 1, key, 0, max_depth);
        }
    }

    // Imprime el numero de veces que aparece key
    printf("Numero de veces que 'key' aparece: %ld\n", nkey);
    printf("Tareas de Conteo Realizadas: %d\n", taskCount);
    printf("Numero de hilos: %d", omp_get_max_threads());

    return 0;
}
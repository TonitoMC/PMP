#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Estructura para almacenar los parámetros que se pasan a cada hilo.
typedef struct {
    int n;
    double result;
} ThreadData;

// Función que calcula el término de la serie para el valor de n correspondiente.
void* calculate_term(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    int n = data->n;
    
    // Calcular el valor del término de la serie
    data->result = pow(-1, n + 1) / n;
    
    // Devolver el resultado como un puntero a double
    double* result_ptr = (double*) malloc(sizeof(double));
    if (result_ptr == NULL) {
        pthread_exit(NULL); // Si falla la asignación de memoria
    }
    *result_ptr = data->result; // Guardar el resultado
    pthread_exit(result_ptr); // Salir devolviendo el resultado
    return NULL;
}

int main() {
    int n_max;

    // Solicitar al usuario que ingrese un valor válido para n_max
    do {
        printf("Ingrese el valor máximo de n para la serie (mayor que 0): ");
        scanf("%d", &n_max);
        if (n_max <= 0) {
            printf("Error: el valor debe ser mayor que 0.\n");
        }
    } while (n_max <= 0);

    // Crear un array para almacenar los identificadores de los hilos y los datos de cada hilo.
    pthread_t* threads = (pthread_t*) malloc(n_max * sizeof(pthread_t));
    ThreadData* thread_data = (ThreadData*) malloc(n_max * sizeof(ThreadData));
    if (threads == NULL || thread_data == NULL) {
        printf("Error al asignar memoria.\n");
        return 1;
    }

    double sum = 0.0;
    int rc;

    // Crear un hilo por cada valor de n.
    for (int i = 0; i < n_max; i++) {
        thread_data[i].n = i + 1;

        // Crear el hilo.
        rc = pthread_create(&threads[i], NULL, calculate_term, (void*) &thread_data[i]);
        if (rc) {
            printf("Error: no se pudo crear el hilo %d. Código de error: %d\n", i, rc);
            free(threads);
            free(thread_data);
            exit(-1);
        }
    }

    // Unir los hilos y sumar los resultados.
    for (int i = 0; i < n_max; i++) {
        double* result_ptr;
        pthread_join(threads[i], (void**) &result_ptr);  // Captura el resultado devuelto
        if (result_ptr != NULL) {
            sum += *result_ptr;  // Sumar el resultado
            free(result_ptr);    // Liberar la memoria asignada
        }
    }

    // Imprimir el resultado final de la serie.
    printf("El resultado de la serie es: %f\n", sum);

    // Limpiar los recursos de memoria asignada.
    free(threads);
    free(thread_data);

    // Terminar el hilo principal de manera ordenada.
    pthread_exit(NULL);
}

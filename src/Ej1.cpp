#include <iostream>
#include <pthread.h>

using namespace std;

// Estructura para los datos de cada Thread
struct ThreadData {
    int n;             // Índice de Fibonacci a calcular
    long long prev1;    // Fib(n-1)
    long long prev2;    // Fib(n-2)
    long long result;   // Resultado del cálculo del número Fibonacci en el índice n
};

// Sumatoria global de todos los números Fibonacci calculados
long long globalSum = 0;

// Mutex para manejar el acceso a la sumatoria global
pthread_mutex_t sumMutex = PTHREAD_MUTEX_INITIALIZER;

// Función para calcular los números de Fibonacci en un rango dado
// Cada thread calcula una parte de la secuencia, usando los valores previos (n-1 y n-2)
void* calculateFibonacci(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    // Mensaje indicando que el hilo comienza su cálculo
    printf("Hilo %lu creado para calcular F%d. Valores recibidos: Fib(n-1) = %lld, Fib(n-2) = %lld\n", 
           pthread_self(), data->n, data->prev1, data->prev2);

    // Calcula el valor de Fibonacci para este thread
    data->result = data->prev1 + data->prev2;

    // Manejo del Mutex
    pthread_mutex_lock(&sumMutex);
    globalSum += data->result; 
    pthread_mutex_unlock(&sumMutex);

    // Imprime el número Fibonacci calculado y la sumatoria global actualizada
    printf("F%d = F%d + F%d = %lld + %lld = %lld. Sumatoria: %lld (Hilo: %lu)\n", 
           data->n, data->n - 1, data->n - 2, data->prev1, data->prev2, data->result, globalSum, pthread_self());

    // Mensaje indicando que el hilo ha terminado su cálculo
    printf("Hilo %lu termino el calculo de F%d. Resultado devuelto: %lld\n", 
           pthread_self(), data->n, data->result);

    return nullptr;
}

int main() {
    int n;

    // Solicitar al usuario el índice de Fibonacci a calcular
    cout << "Ingrese el indice de Fibonacci (<=100, > 0): ";
    cin >> n;

    if (n < 0 || n > 100) {
        cout << "Por favor, ingrese un indice de Fibonacci <= 100 y > 0" << endl;
        return 1;
    }

    if (n == 0){
        printf("Fib(0) = 0, Sumatoria: 0");
        return 0;
    }

    if (n == 1){
        printf("Fib(1) = 1, Sumatoria: 1");
        return 0;
    }

    // Valores iniciales para Fib(0) y Fib(1)
    long long fib0 = 0;
    long long fib1 = 1;

    // Inicializar la sumatoria global con los primeros dos valores de Fibonacci
    globalSum = fib0 + fib1;

    printf("F0 = 0 (calculado por el hilo principal)\n");
    printf("F1 = 1 (calculado por el hilo principal)\n");

    // Arreglo para almacenar los datos de cada thread
    pthread_t threads[n - 1];   // Necesitamos n-1 threads comenzando desde Fib(2)
    ThreadData threadData[n - 1];

    // Crear y ejecutar los threads para calcular desde Fib(2) hasta Fib(n)
    for (int i = 2; i <= n; ++i) {
        threadData[i - 2].n = i;               // Índice de Fibonacci para este thread
        threadData[i - 2].prev1 = fib1;        // Fib(n-1)
        threadData[i - 2].prev2 = fib0;        // Fib(n-2)

        // Crear el thread para calcular Fib(i)
        // printf("Creando hilo para calcular F%d con Fib(n-1) = %lld, Fib(n-2) = %lld\n", 
        //        i, fib1, fib0);
        pthread_create(&threads[i - 2], NULL, calculateFibonacci, &threadData[i - 2]);

        // Esperar a que el thread actual termine
        
        pthread_join(threads[i - 2], NULL);

                // Mensaje indicando que el hilo ha sido cerrado correctamente
        printf("Hilo %lu ha sido cerrado/joined correctamente para F%d\n", 
               threads[i - 2], i);

        // Actualizar Fib(n-2) y Fib(n-1) para el siguiente thread
        fib0 = fib1;
        fib1 = threadData[i - 2].result;
    }

    // Imprimir el resultado final de Fibonacci y la sumatoria total
    cout << "Fib(" << n << ") = " << fib1 << endl;
    cout << "Sumatoria de la secuencia Fibonacci hasta " << n << " = " << globalSum << endl;

    return 0;
}
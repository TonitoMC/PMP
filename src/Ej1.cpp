#include <iostream>
#include <pthread.h>

using namespace std;

// Estructura para pasar parámetros a los hilos
struct ThreadData {
    int iterations;
    int sum;
};

// Función para calcular la serie de Fibonacci hasta el número de iteraciones especificado
void* Fibonacci(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int iterations = data->iterations;

    int a = 0, b = 1, next = 0;
    data->sum = a; // Inicializamos la suma con el primer número

    // Imprimir el primer número de la serie
    if (iterations >= 1) {
        cout << "Iteracion 1: " << a << "\n";
    }

    // Imprimir el segundo número de la serie (si aplica)
    if (iterations >= 2) {
        cout << "Iteracion 2: " << b << "\n";
        data->sum += b; // Agregar el segundo número a la suma
    }

    // Calcular los siguientes números de Fibonacci e imprimirlos
    for (int i = 3; i <= iterations; ++i) {
        next = a + b;
        a = b;
        b = next;
        
        // Imprimir el número de Fibonacci actual
        cout << "Iteracion " << i << ": " << next << "\n";

        // Sumar el número actual a la suma total
        data->sum += next;
    }

    return nullptr;
}
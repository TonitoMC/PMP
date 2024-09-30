#include <iostream>
#include <pthread.h>
#include <unistd.h> // Para la función sleep

using namespace std;

// Variables compartidas
int silo = 0;
int bodega = 0;

// Constante para la cantidad de bolsas de café a producir
const int BAGS_TO_PRODUCE = 400;

// Mutex para sincronizar acceso al silo y bodega
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_silo_full = PTHREAD_COND_INITIALIZER;

// Función para simular la Tostadora 1
void* tostadora1(void* arg) {
    while (true) {
        sleep(1); // Simula el tiempo de producción
        pthread_mutex_lock(&mutex);
        silo++; // Añade 1 lb de café al silo
        cout << "Tostadora 1 produjo: 1 lb de café tostado" << endl;
        cout << "Lbs de café en silo: " << silo << endl;
        pthread_cond_signal(&cond_silo_full); // Despierta a la empacadora si es necesario
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

// Función para simular la Tostadora 2
void* tostadora2(void* arg) {
    while (true) {
        sleep(1); // Simula el tiempo de producción
        pthread_mutex_lock(&mutex);
        silo++; // Añade 1 lb de café al silo
        cout << "Tostadora 2 produjo: 1 lb de café tostado" << endl;
        cout << "Lbs de café en silo: " << silo << endl;
        pthread_cond_signal(&cond_silo_full); // Despierta a la empacadora si es necesario
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

// Función para simular la Empacadora
void* empacadora(void* arg) {
    while (true) {
        pthread_mutex_lock(&mutex);
        // Espera a que haya al menos 5 lbs de café en el silo
        while (silo < 5) {
            pthread_cond_wait(&cond_silo_full, &mutex);
        }
        
        // Empaca 1 bolsa de café y la envía a la bodega
        silo--;
        bodega++;
        cout << "Empacadora produjo: 1 bolsa de 1 lb de café" << endl;
        cout << "Lbs de café en bodega: " << bodega << endl;

        // Si ya se han producido todas las bolsas necesarias, termina el hilo
        if (bodega >= BAGS_TO_PRODUCE) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);
        sleep(1); // Simula el tiempo de empaquetado
    }
    return nullptr;
}

int main() {
    // Hilos para las máquinas
    pthread_t thread_tostadora1, thread_tostadora2, thread_empacadora;

    // Crear hilos
    pthread_create(&thread_tostadora1, nullptr, tostadora1, nullptr);
    pthread_create(&thread_tostadora2, nullptr, tostadora2, nullptr);
    pthread_create(&thread_empacadora, nullptr, empacadora, nullptr);

    // Esperar a que la empacadora termine de producir todas las bolsas
    pthread_join(thread_empacadora, nullptr);

    // Cuando se terminan las bolsas, cancelar las tostadoras
    pthread_cancel(thread_tostadora1);
    pthread_cancel(thread_tostadora2);

    // Mensaje final
    cout << "Lbs de café en silo: " << silo << endl;

    // Destruir el mutex y la variable condicional
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_silo_full);

    return 0;
}

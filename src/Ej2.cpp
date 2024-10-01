#include <iostream>
#include <pthread.h>
#include <unistd.h> // For the sleep function

using namespace std;

// Variables Compartidas
int silo = 0; // Lbs de cafe en el silo
int bodega = 0; // Bolsas empacadas
bool stopProduction = false; // Bandera para parar produccion

// Constante de bolsas de cafe a producir
const int BAGS_TO_PRODUCE = 20;

// Mutex y variable condicional para acceso al silo y la bodega
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_silo_ready = PTHREAD_COND_INITIALIZER;

// Funcion para simular una tostadora
void* tostadora(void* arg) {
    while (true) {
        sleep(1); // Tiempo de produccion

        // Lock mutex
        pthread_mutex_lock(&mutex);

        // Verificar si debemos parar la produccion (ya hay suficientes bolsas)
        if (stopProduction) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Agregar una libra de cafe al silo
        silo++;
        cout << "Tostadora produced: 1 lb of roasted coffee" << endl;
        cout << "Lbs of coffee in silo: " << silo << ", in bodega: " << bodega << endl;

        // Avisar a la empacadora que puede verificar si ya hay 5 libras de cafe listas
        pthread_cond_signal(&cond_silo_ready);

        // Unblock mutex
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

// Funcion para simular la empacadora
void* empacadora(void* arg) {
    while (true) {
        sleep(1); // Simular empaquetado de 1lb de cafe
        // Lock mutex
        pthread_mutex_lock(&mutex);

        // Esperar a que haya 5lbs de cafe minimo y no se ha parado la produccion
        while (silo < 5 && !stopProduction) {
            pthread_cond_wait(&cond_silo_ready, &mutex);
        }

        // Termina el empaquetado si el silo esta vacio y se paro la produccion
        if (stopProduction && silo == 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Empaquetado de libra de cafe
        silo--;
        bodega++;
        cout << "Empacadora produced: 1 bag of 1 lb of coffee" << endl;
        cout << "Lbs of coffee in silo: " << silo << ", in bodega: " << bodega << endl;

        // Cuando se producen suficientes bolsas (silo + bodega) para la produccion / tueste de mas cafe
        if (bodega + silo >= BAGS_TO_PRODUCE) {
            stopProduction = true;
        }

        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

int main() {
    // Threads para las maquinas
    pthread_t thread_tostadora1, thread_tostadora2, thread_empacadora;

    // Creacion de thread indicando que maquina simula
    pthread_create(&thread_tostadora1, nullptr, tostadora, nullptr);
    pthread_create(&thread_tostadora2, nullptr, tostadora, nullptr);
    pthread_create(&thread_empacadora, nullptr, empacadora, nullptr);

    // Join para los 3 threads
    pthread_join(thread_empacadora, nullptr);
    pthread_join(thread_tostadora1, nullptr);
    pthread_join(thread_tostadora2, nullptr);

    // Mensaje final
    cout << "Production complete." << endl;
    cout << "Lbs of coffee in silo: " << silo << endl;
    cout << "Total bags produced: " << bodega << endl;

    // Destruccion de mutex y condvar
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_silo_ready);

    return 0;
}

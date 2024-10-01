#include <iostream>
#include <pthread.h>
#include <unistd.h> // Para la función sleep

using namespace std;

// Variables compartidas
int silo = 0; // Lbs de café en el silo
int bodega = 0; // Total de bolsas empacadas
bool stopProduction = false; // Bandera para detener producción

// Constante para la cantidad de bolsas de café a producir
const int BAGS_TO_PRODUCE = 20;

// Mutex y variable condicional para sincronizar acceso al silo y bodega
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_silo_ready = PTHREAD_COND_INITIALIZER;

// Función para simular una tostadora
void* tostadora(void* arg) {
    while (true) {
        sleep(1); // Simula el tiempo de producción de 1 lb de café
        if (stopProduction){
            break;
        }
        pthread_mutex_lock(&mutex);

        // Añadir 1 lb de café al silo
        silo++;
        cout << "Tostadora produjo: 1 lb de cafe tostado" << endl;
        cout << "Lbs de cafe en silo: " << silo << ", en bodega: " << bodega << endl;

        // Notificar a empacadora de verificar condición de inicio de empaquetado
        pthread_cond_signal(&cond_silo_ready);

        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

// Función para simular la Empacadora
void* empacadora(void* arg) {
    while (true) {
        pthread_mutex_lock(&mutex);
        if (stopProduction == false){
        // Esperar hasta que haya al menos 5 lb de café en el silo
            while (silo < 5 && bodega < BAGS_TO_PRODUCE) {
                pthread_cond_wait(&cond_silo_ready, &mutex);
            }
        }
        sleep(1); // Simula el tiempo de empaquetado

        // Si ya se ha alcanzado la producción total, detener la empacadora
        if (bodega + silo >= BAGS_TO_PRODUCE) {
            stopProduction = true; // Señalar que las tostadoras deben detenerse
            pthread_mutex_unlock(&mutex);
        }
        if (silo == 0){
            break;
        }
        // Empacar 1 lb de café y enviarla a la bodega
        silo--;
        bodega++;
        cout << "Empacadora produjo: 1 bolsa de 1 lb de cafe" << endl;
        cout << "Lbs de cafe en silo: " << silo << ", en bodega: " << bodega << endl;

        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

int main() {
    // Hilos para las máquinas
    pthread_t thread_tostadora1, thread_tostadora2, thread_empacadora;

    // Crear hilos para las tostadoras y empacadora
    pthread_create(&thread_tostadora1, nullptr, tostadora, nullptr);
    pthread_create(&thread_tostadora2, nullptr, tostadora, nullptr);
    pthread_create(&thread_empacadora, nullptr, empacadora, nullptr);

    // Esperar a que la empacadora termine de producir todas las bolsas
    pthread_join(thread_empacadora, nullptr);

    // Esperar a que las tostadoras terminen
    pthread_join(thread_tostadora1, nullptr);
    pthread_join(thread_tostadora2, nullptr);

    // Mensaje final
    cout << "Produccion completa." << endl;
    cout << "Lbs de cafe en silo: " << silo << endl;
    cout << "Total bolsas producidas: " << bodega << endl;

    // Destruir el mutex y la variable condicional
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_silo_ready);

    return 0;
}

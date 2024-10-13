#include <iostream>
#include <pthread.h>
#include <vector>
#include <semaphore.h>
#include <cstdio>

using namespace std;

// Definir saldo inicial de la cuenta
double saldo = 100000.00;

// Cear semáforo para controlar el acceso al cajero
sem_t semaforoCajero;

// Estructura para pasar los datos a cada hilo
struct ClienteData {
    int id;
    double monto;
};

// Función que simula el retiro de dinero por parte de un cliente
void* retirarDinero(void* arg) {
    // Extraer valores de struct
    ClienteData* data = (ClienteData*)arg;
    int id = data->id;
    double monto = data->monto;

    // Imprimir el  inicio de la transaccion
    printf("Cliente %d está intentando retirar Q%.2f\n", id, monto);

    // Esperar a que este disponible el semaforo
    sem_wait(&semaforoCajero);

    // Verificar si el saldo es suficiente para realizar el retiro
    if (monto <= saldo) {
        saldo -= monto;
        printf("Cliente %d retiró Q%.2f. Saldo restante: Q%.2f\n", id, monto, saldo);
    } else {
        printf("Cliente %d no pudo retirar Q%.2f. Saldo insuficiente: Q%.2f\n", id, monto, saldo);
    }

    // Liberar el semáforo para que otro cliente pueda acceder al cajero
    sem_post(&semaforoCajero);

    return NULL;
}

int main() {
    int cantidadClientes;

    // Inicializar el semaforo
    sem_init(&semaforoCajero, 0, 1);

    // Solicitar la cantidad de clientes que van a utilizar el cajero
    printf("Ingrese la cantidad de clientes que van a utilizar el cajero: ");
    cin >> cantidadClientes;

    pthread_t threads[cantidadClientes];
    vector<ClienteData> clientes(cantidadClientes);  // Vector para almacenar los datos de los clientes

    // Solicitar los montos de retiro para cada cliente
    for (int i = 0; i < cantidadClientes; ++i) {
        printf("Ingrese el monto que el cliente %d intentara retirar: ", i+1);
        cin >> clientes[i].monto;
        clientes[i].id = i + 1;
    }

    // Crear los hilos para los clientes
    for (int i = 0; i < cantidadClientes; ++i) {
        pthread_create(&threads[i], NULL, retirarDinero, (void*)&clientes[i]);
    }

    // Esperar a que todos los hilos terminen su ejecución
    for (int i = 0; i < cantidadClientes; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Destruir el semáforo al final
    sem_destroy(&semaforoCajero);

    return 0;
}

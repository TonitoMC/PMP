#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string>  // Include string library

using namespace std;

const char* productos[] = {"Pata", "Respaldo", "Asiento", "Pata", "Pata"};
const int numProductos = 5;  
const int MAX_BUFFER = 5;    
const int MAX_SILLAS = 3;    

bool max_prod = false;
int buffer[MAX_BUFFER];       
int in = 0;                   
int out = 0;                  
int sillasProducidas = 0;    

// Semáforos y mutex
sem_t vacios;   
sem_t llenos;   
pthread_mutex_t mutex;

void* productor(void* arg) {
    int id = *(int*)arg;
    int piezaId;

    while (true) {
        pthread_mutex_lock(&mutex);
        if (max_prod) {
            printf("Productor %d ha dejado de producir, sillas terminadas\n", id);
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        piezaId = rand() % numProductos; // Seleccionar una pieza al azar

        sem_wait(&vacios); // Espera hasta que hay espacio en el buffer
        pthread_mutex_lock(&mutex); // Protege el acceso al buffer

        if (max_prod) { // Check again under mutex before producing
            pthread_mutex_unlock(&mutex);
            sem_post(&vacios); // Prevent deadlock by releasing semaphore
            break;
        }

        buffer[in] = piezaId; // Añade la pieza al buffer
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la coloco en la posicion " << in << endl;
        in = (in + 1) % MAX_BUFFER; // Avanza el índice circular del buffer

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos); // Incrementa el número de productos disponibles
        
        sleep(1); // Simula el tiempo de fabricación
    }

    return NULL;
}

void* consumidor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    int patas = 0, asientos = 0, respaldos = 0;

    while (true) {
        pthread_mutex_lock(&mutex);
        if (sillasProducidas >= MAX_SILLAS) {
            max_prod = true;
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        sem_wait(&llenos); // Espera hasta que existan productos disponibles
        pthread_mutex_lock(&mutex); // Protege el acceso al buffer

        if (max_prod) { // Additional safety check under mutex
            pthread_mutex_unlock(&mutex);
            sem_post(&llenos); // Prevent deadlock by releasing semaphore
            break;
        }

        piezaId = buffer[out];
        string pieza = productos[piezaId]; // Get piece type

        cout << "Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posicion " << out << endl;
        out = (out + 1) % MAX_BUFFER; // Avanza en el índice circular del buffer

        if (pieza == "Pata") {
            patas++;
        } else if (pieza == "Respaldo") {
            respaldos++;
        } else if (pieza == "Asiento") {
            asientos++;
        }

        if (patas >= 4 && asientos >= 1 && respaldos >= 1) {
            sillasProducidas++;
            cout << "Consumidor " << id << " ha ensamblado una silla completa. Sillas ensambladas: "
                 << sillasProducidas << "/" << MAX_SILLAS << endl;
            patas -= 4;
            asientos--;
            respaldos--;
        }

        printf("Consumidor %d. Patas: %d, Asientos: %d, Respaldos %d\n", id, patas, asientos, respaldos);

        pthread_mutex_unlock(&mutex);
        sem_post(&vacios); // Incrementa el número de espacios vacíos

        sleep(2); // Simula el tiempo de ensamblaje
    }

    return NULL;
}

int main() {
    int numProductores, numConsumidores;

    // Solicitar la cantidad de productores y consumidores
    cout << "Ingrese el numero de productores: ";
    cin >> numProductores;
    cout << "Ingrese el numero de consumidores: ";
    cin >> numConsumidores;

    pthread_t productores[100], consumidores[100];  
    int idProductores[100], idConsumidores[100];    

    // Inicializa semáforos y mutex
    sem_init(&vacios, 0, MAX_BUFFER);  
    sem_init(&llenos, 0, 0);           
    pthread_mutex_init(&mutex, NULL);

    // Crea hilos productores
    for (int i = 0; i < numProductores; ++i) {
        idProductores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &idProductores[i]);
    }

    // Crea hilos consumidores
    for (int i = 0; i < numConsumidores; ++i) {
        idConsumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &idConsumidores[i]);
    }

    // Espera a que los hilos terminen
    for (int i = 0; i < numProductores; ++i) {
        pthread_join(productores[i], NULL);
    }

    for (int i = 0; i < numConsumidores; ++i) {
        pthread_join(consumidores[i], NULL);
    }

    // Destruye semáforos y mutex
}
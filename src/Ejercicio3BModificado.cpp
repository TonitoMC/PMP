#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string>

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

// Semaforos y Mutex
sem_t vacios;   
sem_t llenos;   
pthread_mutex_t mutex;
pthread_mutex_t maxprodMutex;

// Rutina de productor
void* productor(void* arg) {
    int id = *(int*)arg;
    int piezaId;

    while (true) {
        piezaId = rand() % numProductos; // Selecciona una pieza aleatoria

        sem_wait(&vacios); // Espera a que haya espacio en el buffer
        pthread_mutex_lock(&maxprodMutex);

        // Revisar si puede continuar produciendo
        if (max_prod) {
            pthread_mutex_unlock(&maxprodMutex);
            sem_post(&vacios);
            break;
        }
        pthread_mutex_unlock(&maxprodMutex);

        pthread_mutex_lock(&mutex);

        // Agrega la pieza al buffer
        buffer[in] = piezaId;
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la coloco en la posicion " << in << endl;
            
        // Mueve el indice del buffer
        in = (in + 1) % MAX_BUFFER;

        pthread_mutex_unlock(&mutex);

        sem_post(&llenos); // Notificar que hay un item disponible
        
        sleep(1);
    }
    return NULL;
}

// Rutina de consumidor
void* consumidor(void* arg) {
    int id = *(int*)arg;
    int piezaId;

    // Contador de piezas de cada consumidor
    int patas = 0, asientos = 0, respaldos = 0;

    while (true) {
        pthread_mutex_lock(&maxprodMutex);
        if (max_prod) { // Revision nuevamente de condicion de finalizacion
            pthread_mutex_unlock(&maxprodMutex);
            sem_post(&llenos);
            break;
        }
        pthread_mutex_unlock(&maxprodMutex);
        sem_wait(&llenos); // Espera a que haya productos disponibles

        pthread_mutex_lock(&maxprodMutex);

        if (max_prod) { // Revision nuevamente de condicion de finalizacion
            pthread_mutex_unlock(&maxprodMutex);
            sem_post(&llenos);
            break;
        }
        pthread_mutex_unlock(&maxprodMutex);

        pthread_mutex_lock(&mutex);
        piezaId = buffer[out];
        string pieza = productos[piezaId];

        cout << "Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posicion " << out << endl;
        out = (out + 1) % MAX_BUFFER; // Movimiento del indice del buffer
        pthread_mutex_unlock(&mutex);

        // Verificacion de tipo de pieza
        if (pieza == "Pata") {
            patas++;
        } else if (pieza == "Respaldo") {
            respaldos++;
        } else if (pieza == "Asiento") {
            asientos++;
        }


        // Verificacion de habilidad de producir silla
        if (patas >= 4 && asientos >= 1 && respaldos >= 1) {
            pthread_mutex_lock(&maxprodMutex);
            sillasProducidas++;
            if (sillasProducidas >= MAX_SILLAS){
                max_prod = true;
                sem_post(&vacios); // Aumenta la cantidad de espacios vacios
                pthread_mutex_unlock(&maxprodMutex);
                break;
            }
            pthread_mutex_unlock(&maxprodMutex);

            cout << "Consumidor " << id << " ha ensamblado una silla completa. Sillas ensambladas: "
                 << sillasProducidas << "/" << MAX_SILLAS << endl;
            patas -= 4;
            asientos--;
            respaldos--;
        }

        printf("Consumidor %d. Patas: %d, Asientos: %d, Respaldos %d\n", id, patas, asientos, respaldos);

        sem_post(&vacios); // Aumenta la cantidad de espacios vacios

        sleep(2); // Simula tiempo de produccion
    }

    printf("Consumidor %d dejo de producir. Patas: %d, Asientos: %d, Respaldos %d\n", id, patas, asientos, respaldos);
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
    pthread_mutex_init(&maxprodMutex, NULL);

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

    // Impresion de items del buffer
    cout << "\nItems en el Buffer" << endl;
    for (int i = 0; i < MAX_BUFFER; ++i) {
        cout << "Buffer[" << i << "]: " << productos[buffer[i]] << endl;
    }
    
    // Destruye semáforos y mutex
    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&maxprodMutex);

    return 0;
}

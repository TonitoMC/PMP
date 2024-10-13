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

// Track the number of pieces each consumer has consumed
int consumerPatas[100] = {0};
int consumerRespaldo[100] = {0};
int consumerAsiento[100] = {0};

// Semaphores and mutex
sem_t vacios;   
sem_t llenos;   
pthread_mutex_t mutex;

// Producer function
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

        piezaId = rand() % numProductos; // Select a random piece

        sem_wait(&vacios); // Wait until there is space in the buffer
        pthread_mutex_lock(&mutex); // Protect access to the buffer

        if (max_prod) {
            pthread_mutex_unlock(&mutex);
            sem_post(&vacios); // Prevent deadlock by releasing semaphore
            break;
        }

        buffer[in] = piezaId; // Add the piece to the buffer
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la coloco en la posicion " << in << endl;
        in = (in + 1) % MAX_BUFFER; // Move the circular buffer index

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos); // Increase the number of available products
        
        sleep(1); // Simulate production time
    }

    return NULL;
}

// Consumer function
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

        sem_wait(&llenos); // Wait until there are available products
        pthread_mutex_lock(&mutex); // Protect access to the buffer

        if (max_prod) {
            pthread_mutex_unlock(&mutex);
            sem_post(&llenos); // Prevent deadlock by releasing semaphore
            break;
        }

        piezaId = buffer[out];
        string pieza = productos[piezaId]; // Get the piece type

        cout << "Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posicion " << out << endl;
        out = (out + 1) % MAX_BUFFER; // Move the circular buffer index

        if (pieza == "Pata") {
            patas++;
            consumerPatas[id]++;  // Track how many patas this consumer processed
        } else if (pieza == "Respaldo") {
            respaldos++;
            consumerRespaldo[id]++;  // Track how many respaldos this consumer processed
        } else if (pieza == "Asiento") {
            asientos++;
            consumerAsiento[id]++;  // Track how many asientos this consumer processed
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
        sem_post(&vacios); // Increase the number of empty spaces

        sleep(2); // Simulate assembly time
    }

    return NULL;
}

// Function to print the final report
void printFinalReport(int numConsumidores) {
    cout << "\n=== Final Report ===" << endl;
    
    // Report remaining items in the buffer
    cout << "Remaining items in the buffer: " << endl;
    for (int i = 0; i < MAX_BUFFER; i++) {
        cout << "Position " << i << ": " << productos[buffer[i]] << endl;
    }
    
    // Report each consumer's processed items
    for (int i = 0; i < numConsumidores; ++i) {
        cout << "\nConsumidor " << i + 1 << " final stats:" << endl;
        cout << "  Patas: " << consumerPatas[i + 1] << endl;
        cout << "  Respaldos: " << consumerRespaldo[i + 1] << endl;
        cout << "  Asientos: " << consumerAsiento[i + 1] << endl;
    }
    cout << "====================" << endl;
}

int main() {
    int numProductores, numConsumidores;

    // Request the number of producers and consumers
    cout << "Ingrese el numero de productores: ";
    cin >> numProductores;
    cout << "Ingrese el numero de consumidores: ";
    cin >> numConsumidores;

    pthread_t productores[100], consumidores[100];  
    int idProductores[100], idConsumidores[100];    

    // Initialize semaphores and mutex
    sem_init(&vacios, 0, MAX_BUFFER);  
    sem_init(&llenos, 0, 0);           
    pthread_mutex_init(&mutex, NULL);

    // Create producer threads
    for (int i = 0; i < numProductores; ++i) {
        idProductores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &idProductores[i]);
    }

    // Create consumer threads
    for (int i = 0; i < numConsumidores; ++i) {
        idConsumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &idConsumidores[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < numProductores; ++i) {
        pthread_join(productores[i], NULL);
    }

    for (int i = 0; i < numConsumidores; ++i) {
        pthread_join(consumidores[i], NULL);
    }

    // Print the final report
    printFinalReport(numConsumidores);

    // Destroy semaphores and mutex
    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);

    return 0;
}

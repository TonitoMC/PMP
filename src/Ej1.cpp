#include <stdio.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <unistd.h> // Para la función sleep

using namespace std;

// Struct para almacenar datos relacionados a cada item del menu
struct foodItem {
    string name;
    double itemPrice;
    double itemCost;
    int itemsSold;
    double itemSales;
    double itemUtility;
    int threadNum; // Para identificar el número de hilo
};

// Variables compartidas para totales
double totalSalesJuly = 0;
double totalUtilityJuly = 0;
double totalSalesAugust = 0;
double totalUtilityAugust = 0;

// Mutex para sincronización
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Función para que cada hilo calcule las ventas y utilidad de un ítem de Julio
void* calculateJuly(void* arg) {
    foodItem* item = (foodItem*)arg;
    
    // Indicar que el hilo ha comenzado sus cálculos
    cout << "Calculo Hilo " << item->threadNum << " Iniciando (Julio)" << endl;
    
    // Simular tiempo de cálculo
    sleep(1);
    
    item->itemSales = item->itemPrice * item->itemsSold;
    item->itemUtility = (item->itemPrice - item->itemCost) * item->itemsSold;

    // Bloquear mutex al actualizar totales
    pthread_mutex_lock(&mutex);
    totalSalesJuly += item->itemSales;
    totalUtilityJuly += item->itemUtility;
    pthread_mutex_unlock(&mutex); // Desbloquear mutex después de actualizar
    
    // Indicar que el hilo ha terminado sus cálculos
    cout << "Calculo Hilo " << item->threadNum << " Terminado (Julio)" << endl;

    return nullptr;
}

// Función para que cada hilo calcule las ventas y utilidad de un ítem de Agosto
void* calculateAugust(void* arg) {
    foodItem* item = (foodItem*)arg;

    // Indicar que el hilo ha comenzado sus cálculos
    cout << "Calculo Hilo " << item->threadNum << " Iniciando (Agosto)" << endl;

    // Simular tiempo de cálculo
    sleep(1);
    
    item->itemSales = item->itemPrice * item->itemsSold;
    item->itemUtility = (item->itemPrice - item->itemCost) * item->itemsSold;

    // Bloquear mutex al actualizar totales
    pthread_mutex_lock(&mutex);
    totalSalesAugust += item->itemSales;
    totalUtilityAugust += item->itemUtility;
    pthread_mutex_unlock(&mutex); // Desbloquear mutex después de actualizar

    // Indicar que el hilo ha terminado sus cálculos
    cout << "Calculo Hilo " << item->threadNum << " Terminado (Agosto)" << endl;

    return nullptr;
}

int main() {
    // Inicialización de datos para Julio
    foodItem july[8] = {
        {"Pastel de Chocolate", 60.00, 20.00, 300, 0, 0, 1},
        {"White Mocha", 32.00, 19.20, 400, 0, 0, 2},
        {"Cafe Americano", 22.00, 13.20, 1590, 0, 0, 3},
        {"Latte", 24.00, 17.20, 200, 0, 0, 4},
        {"Toffee Coffee", 28.00, 20.10, 390, 0, 0, 5},
        {"Capuccino", 24.00, 17.20, 1455, 0, 0, 6},
        {"Smores Latte", 32.00, 23.00, 800, 0, 0, 7},
        {"Cafe Tostado", 60.00, 20.00, 60, 0, 0, 8}
    };

    // Inicialización de datos para Agosto
    foodItem august[8] = {
        {"Pastel de Chocolate", 60.00, 20.00, 250, 0, 0, 9},
        {"White Mocha", 32.00, 19.20, 380, 0, 0, 10},
        {"Cafe Americano", 22.00, 13.20, 800, 0, 0, 11},
        {"Latte", 24.00, 17.20, 250, 0, 0, 12},
        {"Toffee Coffee", 28.00, 20.10, 600, 0, 0, 13},
        {"Capuccino", 24.00, 17.20, 1200, 0, 0, 14},
        {"Smores Latte", 32.00, 23.00, 1540, 0, 0, 15},
        {"Cafe Tostado", 60.00, 20.00, 15, 0, 0, 16}
    };
    
    // Array para almacenar los 16 hilos
    pthread_t threads[16];
    
    // Creación de hilos para Julio
    for (int i = 0; i < 8; ++i) {
        if (pthread_create(&threads[i], nullptr, calculateJuly, (void*)&july[i])) {
            cerr << "Error creando hilo para ítem de Julio: " << july[i].name << endl;
            return 1;
        }
    }
    
    // Creación de hilos para Agosto
    for (int i = 0; i < 8; ++i) {
        if (pthread_create(&threads[i + 8], nullptr, calculateAugust, (void*)&august[i])) {
            cerr << "Error creando hilo para ítem de Agosto: " << august[i].name << endl;
            return 1;
        }
    }
    
    // Join para los 16 hilos
    for (int i = 0; i < 16; ++i) {
        if (pthread_join(threads[i], nullptr)) {
            cerr << "Error uniendo hilo." << endl;
            return 2;
        }
    }
    
    // Imprimir ventas y utilidad por producto en Julio
    cout << "REPORTE DEL MES DE JULIO\n --- MONTO DE VENTAS POR PRODUCTO ---" << endl;
    for (const auto& item : july) {
        cout << item.name << " : Q" << item.itemSales << endl;
    }

    cout << "\n--- UTILIDAD POR PRODUCTO ---" << endl;
    for (const auto& item : july) {
        cout << item.name << " : Q" << item.itemUtility << endl;
    }

    // Imprimir total de ventas y utilidad de Julio
    cout << "\nTOTAL VENTAS JULIO: Q" << totalSalesJuly << endl;
    cout << "TOTAL UTILIDAD JULIO: Q" << totalUtilityJuly << endl;

    // Imprimir ventas y utilidad por producto en Agosto
    cout << "\nREPORTE DEL MES DE AGOSTO\n --- MONTO DE VENTAS POR PRODUCTO ---" << endl;
    for (const auto& item : august) {
        cout << item.name << " : Q" << item.itemSales << endl;
    }

    cout << "\n--- UTILIDAD POR PRODUCTO ---" << endl;
    for (const auto& item : august) {
        cout << item.name << " : Q" << item.itemUtility << endl;
    }

    // Imprimir total de ventas y utilidad de Agosto
    cout << "\nTOTAL VENTAS AGOSTO: Q" << totalSalesAugust << endl;
    cout << "TOTAL UTILIDAD AGOSTO: Q" << totalUtilityAugust << endl;
    
    // Destruir el mutex
    pthread_mutex_destroy(&mutex);

    return 0;
}

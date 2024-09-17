//Ejercicio 2 laboratorio 6
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 10

void *PrintHello(void *paramID) {
    int *id;
    id = (int *)paramID;
    printf("Hello world thread No. %d!\n", *id);
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t threadsID[NUM_THREADS];
    int param[NUM_THREADS];
    int rc, t;
    int option;

    printf("Escoge una opcion: \n");
    printf("1. Crear hilos y hacer join en ciclos separados (Parte B)\n");
    printf("2. Crear y hacer join de cada hilo en el mismo ciclo (Parte C)\n");
    scanf("%d", &option);
//Parte B
    if (option == 1) {
        //Crear hilos y hacer join en ciclos separados
        for (t = 0; t < NUM_THREADS; t++) {
            param[t] = t;
            printf("In main: creating thread %d\n", t);
            rc = pthread_create(&threadsID[t], NULL, PrintHello, (void *)&param[t]);
            if (rc) {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
        for (t = 0; t < NUM_THREADS; t++) {
            pthread_join(threadsID[t], NULL);
        }
//Parte C
    } else if (option == 2) {
        //Crear hilos y usar join en un ciclo
        for (t = 0; t < NUM_THREADS; t++) {
            param[t] = t;
            printf("In main: creating and joining thread %d\n", t);
            rc = pthread_create(&threadsID[t], NULL, PrintHello, (void *)&param[t]);
            if (rc) {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
            pthread_join(threadsID[t], NULL);
        }
    } else {
        printf("elige una opcion valida\n");
    }

    pthread_exit(NULL);
}

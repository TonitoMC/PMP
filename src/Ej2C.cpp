/*
*-----------------------------------------------------------
* HelloWorld.cpp
*-----------------------------------------------------------
* UNIVERSIDAD DEL VALLE DE GUATEMALA
* FACULTAD DE INGENIERÍA
* DEPARTAMENTO DE CIENCIA DE LA COMPUTACIÓN

* CC3086 - Programacion de Microprocesadores
*
*-----------------------------------------------------------
* Descripción: demostración de operaciones básicas para 
* creación de hilos posix.
*-----------------------------------------------------------
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 10

/* -----------------------------------------------------------
** Subrutina *void usada por el pthread, luego de crearse.
** Los parámetros son punteros a void, debido a que los threads
** emplean referencias de memoria: la información pasa por 
** referencia indirecta
** ---------------------------------------------------------*/
void *PrintHello(void *paramID) {
	int *id;						// puntero a entero
	id = (int *)paramID;			// cast de conversión de tipo void * a int *
	
	// *tid desreferencia el puntero = accede al valor del entero que está almacenado 
	// en la dirección de memoria a la que tid apunta. 
	// *tid obtiene el valor del entero al que tid está apuntando.
	printf("Hello World! from thread #%d!\n", *id);
	pthread_exit(NULL);
	return NULL; 
}

int main(int argc, char *argv[]) {
	// Usar intptr_t para identificar hilos de manera segura
	pthread_t threadsID[NUM_THREADS];
	
	// Parametro a compartir equivalente al num del n hilo
	int rc, t, param[NUM_THREADS];
	
	// Cada hilo se crea individualmente
	for (t=0; t< NUM_THREADS; t++) {
		printf("In main: creating thread %d\n", t);
		param[t] = t;
		/* -----------------------------------------------------------
		** Al crear un hilo, se pasan los  siguientes parametros:	
		** el valor del ID, el valor del atributo , la subrutina a
		** ejecutar por el hilo y el puntero para el parametro de 
		** la subrutina:
		**
		** pthread_create(&id,NULL,subrutine,(void *)parameter
		** ---------------------------------------------------------*/
		rc = pthread_create(&threadsID[t], NULL, PrintHello, (void *)&param[t]);

		// Si algún hilo no pudo crearse, muestra mensaje de error
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		pthread_join(rc, NULL);
	}
	pthread_exit(NULL);	
}
#include <stdio.h>
#include <omp.h>

int main() {
    // Definicion de Variables
    int variable1 = 0;
    int variable2 = 0;
    int n = 50;

    // Inicializacion region paralela con variable1 compartida y variable2 privada
    #pragma omp parallel shared(variable1) private(variable2)
    {
        variable2 = 0; // Declaramos variable2 dentro de la región paralela
        #pragma omp for
        for (int i = 0; i < n; i++) {
        // Modificacion de variables
        #pragma omp atomic
        variable1 += i; //Variable1 Atomic para evitar comportamientos no deseados al ser compartida

        variable2 += i;
        // Mostramos los valores de las variables en cada iteración
        printf("Thread %d, iter %d: variable1 = %d, variable2 = %d\n", omp_get_thread_num(), i, variable1, variable2);
    }
    }

    printf("Final: variable1 = %d, variable2 = %d\n", variable1, variable2);

    return 0;
}

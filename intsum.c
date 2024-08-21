#include <stdio.h>
#include <omp.h>

int intSum(int n){
    int intSum = 0;
    #pragma omp parallel
    {
        #pragma omp for reduction(+:intSum)
        for (int i = 0; i <= n; i++){
            intSum += i;
        }
    }
    return intSum;
};

int main(){
    int result = intSum(1000000);
    printf("%d", result);

}
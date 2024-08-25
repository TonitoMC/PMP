#include <stdio.h>
#include <omp.h>
#define N 131072

long count_key(long Nlen, long *a, long key)
{
    long count = 0;
    for (int i = 0; i < Nlen; i++)
        if (a[i] == key)
            count++;
    return count;
}

int main()
{
    long a[N], key = 42, nkey = 0;
    for (long i = 0; i < N; i++)
        a[i] = rand() % N;
    a[N % 43] = key;
    a[N % 73] = key;
    a[N % 3] = key;

    nkey = count_key(N, a, key); // cuenta key secuencialmente
    printf("Numero de veces que 'key' aparece secuencialmente: %ld\n", nkey);

    return 0;
}
#include <libprivsep_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main (void){
    int i = 0;
    int *p,*c,*z,*f;
    clock_t start_t, end_t, total_t;
    start_t = clock();
    for (i=0;i<100000000; i++)
    {
        p = (int *) malloc(2*4096);
        free(p);
        p = (int *) malloc(2*4096);
        c = (int *) malloc(2*4096);
        z = (int *) malloc(2*4096);
        f = (int *) malloc(2*4096);
        free(p);
        free(c);
        free(z);
        free(f);
    }
    end_t = clock();
    total_t = (double) (end_t - start_t)/CLOCKS_PER_SEC;
    printf ("Total time taken by CPU: %ld\n", total_t);
    int *p1 = malloc(sizeof(int)*30);
    int *p2 = malloc(sizeof(char));
    if (p1 != NULL && p2 != NULL){
        *p1=10;
        *p2=+3;
        printf("%p = %d \n%p = %d\n", (int *)p1,*((int *)p1),(int *)p2, *((int *)p2));
    }
    free(p1);
    free(p2);

    return 1;
}

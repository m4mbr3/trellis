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
        p = (int *) privsep_malloc(8182, 9);
        privsep_free(p);
        p = (int *) privsep_malloc(8182, 9);
        c = (int *) privsep_malloc(8182, 9);
        z = (int *) privsep_malloc(8182, 9);
        f = (int *) privsep_malloc(8182, 9);
        privsep_free(p);
        privsep_free(c);
        privsep_free(z);
        privsep_free(f);
    }
    end_t = clock();
    total_t = (double) (end_t - start_t)/CLOCKS_PER_SEC;
    printf ("Total time taken by CPU: %ld\n", total_t);
    int *p1 = privsep_malloc(sizeof(int)*30, 9);
    int *p2 = privsep_malloc(sizeof(char), 9);
    if (p1 != NULL && p2 != NULL){
        *p1=10;
        *p2=+3;
        printf("%p = %d \n%p = %d\n", (int *)p1,*((int *)p1),(int *)p2, *((int *)p2));
    }
    privsep_free(p1);
    privsep_free(p2);

    return 1;
}

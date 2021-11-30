//
// Created by Binny Friedman on 29/10/2021.
//
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct { int a; int b; } myarg_t;
typedef struct { int x; int y; } myret_t;

void *myThread(void *arg){
myarg_t *args = (myarg_t *) arg;
    printf("%d %d\n", args->a, args->b);
    myret_t oops;
    oops.x = 1;
    oops.y = 3;
    return (void *) &oops;
}

int main(int argc,char **argv){
    pthread_t p;
    myret_t *rValues;
    myarg_t args = {10,20};
    pthread_create(&p,NULL,myThread,&args);
    pthread_join(p,(void **) &rValues);
    printf("returned %d %d\n",rValues->x,rValues->y);
    free(rValues);
    return 0;
}

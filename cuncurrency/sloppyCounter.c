#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#define  THRESHOLD 10000

typedef struct {
    int value;
    pthread_mutex_t lock;
} counter_t;

typedef struct {
    int start;
    int end;
} measure_t;



counter_t counter;

int getTime(){
    struct timeval t;
    gettimeofday(&t,NULL);
}

void init_measure(measure_t *m){
    m->start = getTime();
}


void end_measure(measure_t *m){
    m->end = getTime();
}

void counter_init(counter_t *c) {
    c->value = 0;
    pthread_mutex_init(&c->lock, NULL);
}

void counter_increment(counter_t *c) {
    pthread_mutex_lock(&c->lock);
    c->value++;
    pthread_mutex_unlock(&c->lock);
}

void counter_decrement(counter_t *c) {
    pthread_mutex_lock(&c->lock);
    c->value--;
    pthread_mutex_unlock(&c->lock);
}

int getCounterValue(counter_t *c) {
    pthread_mutex_lock(&c->lock);
    int value = c->value;
    pthread_mutex_unlock(&c->lock);

    return value;
}
void sloppyIncrement(counter_t *c,int by) {
    pthread_mutex_lock(&c->lock);
    c->value += by;
    pthread_mutex_unlock(&c->lock);
}

void *thread_func(void *arg) {
    int local = 0;
    for(int i = 0; i < 10000; i++) {
        if(local > THRESHOLD) {
            sloppyIncrement(&counter,local);
            local = 0;
        }
        local++;
    }
}


int init_threads(){
    pthread_t t1, t2,t3,t4,t5;


    pthread_create(&t1,NULL,thread_func,NULL);

    pthread_create(&t2,NULL,thread_func,NULL);

    pthread_create(&t3,NULL,thread_func,NULL);

    pthread_create(&t4,NULL,thread_func,NULL);

    pthread_create(&t5,NULL,thread_func,NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);
}

void operation(th){
    counter_init(&counter);
    init_threads();

}




int main(int argc,char **argv){
    measure_t m;
    init_measure(&m);
    operation();
    end_measure(&m);
    printf('Time taken %d\n',m.end-m.start);
    printf('Counter value %d\n',getCounterValue(&counter));
    return 0;
}


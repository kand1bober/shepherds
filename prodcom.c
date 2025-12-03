#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define kBufSz 128

typedef struct {
    pthread_mutex_t m;
    pthread_cond_t full; //shows producer, that he can fill the buf
    pthread_cond_t empty; //shows consumer, that he can empty the buf
    
    char buf[kBufSz];
    int buf_sz;
} Monitor;

bool mon_buf_is_full(Monitor* mon)
{
    return (mon->buf_sz == kBufSz);  
}

bool mon_buf_is_empty(Monitor* mon)
{   
    return (mon->buf_sz == 0);
}

void mon_buf_write(Monitor* mon, char* str)
{
    int sz = strlen(str);
    snprintf(mon->buf, sz + 1, "%s", str);
    mon->buf_sz = sz + 1;
}

void mon_buf_read(Monitor* mon, char* res)
{
    snprintf(res, mon->buf_sz, "%s", mon->buf);
    mon->buf_sz = 0;
}

void* producer(void* monitor)
{
    Monitor* mon = (Monitor*)monitor;
    pthread_mutex_lock(&mon->m); //take mutex

    while (mon_buf_is_full(mon)) {
        pthread_cond_wait(&mon->empty, &mon->m); //wait and free mutex if buf is not ready
    }
    
    //if buf is ready => enter the critical setion (fill the buf)
    char str[128] = {};
    printf("producer:\n");
    scanf("%s", str);
    mon_buf_write(monitor, str);

    pthread_cond_signal(&mon->full); //signal the consumer to begin consuming
    pthread_mutex_unlock(&mon->m); //leave mutex
    return NULL;
}

void* consumer(void* monitor)
{
    Monitor* mon = (Monitor*)monitor;
    pthread_mutex_lock(&mon->m); //take mutex

    while (mon_buf_is_empty(mon)) {
        pthread_cond_wait(&mon->full, &mon->m); //wait and free mutex if buf is not ready for emptying
    }
    
    //if buf is ready => enter the critical setion (empty the buf)
    char str[128] = {};
    mon_buf_read(monitor, str);
    printf("consumer: '%s'", str);

    pthread_cond_signal(&mon->empty); //signal the producer to begin producing
    pthread_mutex_unlock(&mon->m); //leave mutex

    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t prod, cons;
    Monitor monitor = {};

    pthread_create(&prod, NULL, producer, (void*)&monitor);
    pthread_create(&cons, NULL, consumer, (void*)&monitor);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    return 0;
}

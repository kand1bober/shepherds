#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Направления
#define LEFT_TO_RIGHT 0
#define RIGHT_TO_LEFT 1

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;

} MonitorBridge;

int monitor_()
{

}

void* shepherd_thread(void* arg) {
    shepherd(*(int*)arg);
    return NULL;
}

void left_shepherd(int num)
{
    
}

void right_shepherd(int num)
{

}

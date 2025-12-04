#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define kRightDir 1
#define kLeftDir 2

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t walk_right, 
                   walk_left, 
                   hat;

    bool bridge_is_busy;
    int queue_count; 

} BridgeMonitor;

typedef struct {
    int num;
    BridgeMonitor* monitor;
} Shepherd;

bool mon_bridge_is_busy(BridgeMonitor* mon)
{
    return mon->bridge_is_busy;
}

void cross_bridge(BridgeMonitor* mon, int num, int dir)
{
    mon->bridge_is_busy = true;

    // crossing
    printf("shepherd #%d: crossing (%s)\n", num, (dir == kRightDir) ? "=>" : "<=");
    usleep(1000000);

    printf("shepherd: #%d: finished crossing\n", num);
    mon->bridge_is_busy = false;
}

void* left_shepherd(void* arg)
{
    int num = ((Shepherd*)arg)->num;
    BridgeMonitor* mon = ((Shepherd*)arg)->monitor;

    printf("shepherd #%d: came\n", num);

    pthread_mutex_lock(&mon->mutex);

    while (mon_bridge_is_busy(mon)) {
        pthread_cond_wait(&mon->walk_right, &mon->mutex);
    }

    cross_bridge(mon, num, kRightDir);
    
    pthread_cond_signal(&mon->walk_left);
    pthread_mutex_unlock(&mon->mutex);

    return NULL;
}

void* right_shepherd(void* arg)
{
    int num = ((Shepherd*)arg)->num;
    BridgeMonitor* mon = ((Shepherd*)arg)->monitor;

    printf("shepherd #%d: came\n", num);

    pthread_mutex_lock(&mon->mutex);
    while (mon_bridge_is_busy(mon)) {
        pthread_cond_wait(&mon->walk_left, &mon->mutex);
    }

    cross_bridge(mon, num, kLeftDir);    

    pthread_cond_signal(&mon->walk_right);
    pthread_mutex_unlock(&mon->mutex);

    return NULL;
}

int main()
{
    //set threads
    pthread_t t_arr[10] = {0};
    BridgeMonitor monitor = {};
    Shepherd shepherd_arr[10];
    for (int i = 0; i < 10; i++) { 
        shepherd_arr[i].monitor = &monitor;
        shepherd_arr[i].num = i;
    }

    //start threads
    for (int i = 0; i < 4; i++) {
        shepherd_arr[i].num = i;
        pthread_create(&(t_arr[i]), NULL, left_shepherd, (void*)(&shepherd_arr[i]));
        usleep(100);
    }
    for (int i = 4; i < 7; i++) {
        shepherd_arr[i].num = i;
        pthread_create(&(t_arr[i]), NULL, right_shepherd, (void*)(&shepherd_arr[i]));
        usleep(100);
    }
    for (int i = 7; i < 10; i++) {
        shepherd_arr[i].num = i;
        pthread_create(&(t_arr[i]), NULL, left_shepherd, (void*)(&shepherd_arr[i]));
        usleep(100);
    }

    //join threads
    for (int i = 0; i < 10; i++) {
        pthread_join(t_arr[i], NULL);
    }

    return 0;
}
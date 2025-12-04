#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define kTimeToCrossAlone 10 //usec
#define kTimeToCrossWithHerd 30 //usec

#define kRightDir 1
#define kLeftDir 2

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t walk_right, //condition variables 
                   walk_left;

    pthread_cond_t ready_to_walk_right;
    pthread_cond_t left_crossed_breadge;
    bool shepherd_crossing_alone; //to check that only one left shepherd leaves his hat

    bool hat_on_right;
    bool bridge_is_busy; //shared resource
    
    int right_queue_count, //counters of shepherd queues
        left_queue_count; 

} BridgeMonitor;

typedef struct {
    int num;
    BridgeMonitor* monitor;
} Shepherd;

void alone_cross_bridge_and_leave_hat(BridgeMonitor* mon, int num, bool* left_hat)
{
    mon->shepherd_crossing_alone = true;
    printf("shepherd #%d: crossing (=>)\n", num);
    usleep(kTimeToCrossAlone);
    
    mon->hat_on_right = true;
    *left_hat = true;
    printf("shepherd #%d: left hat on rigt side\n", num);

    printf("shepherd #%d: crossing (<=)\n", num);
    usleep(kTimeToCrossAlone);
}

void herd_cross_bridge(BridgeMonitor* mon, int num, int dir, bool left_hat)
{
    mon->bridge_is_busy = true;

    // crossing
    printf("shepherd + herd #%d: crossing (%s)\n", num, (dir == kRightDir) ? "=>" : "<=");
    usleep(kTimeToCrossWithHerd);

    printf("shepherd + herd #%d: finished crossing\n", num);

    if (dir == kRightDir) { 
        if (left_hat) {
            while (mon->left_queue_count > 1) { //wait for left shepherds to to cross
                mon->bridge_is_busy = false;
                pthread_cond_wait(&mon->left_crossed_breadge, &mon->mutex);
            }  
            mon->bridge_is_busy = true;

            pthread_cond_signal(&mon->walk_left); //signal condition for right side shepherds
            mon->hat_on_right = false;
            printf("shepherd #%d: took my hat\n", num);
            mon->left_queue_count--;
        }
        else {  
            mon->left_queue_count--;
            if (mon->left_queue_count == 1) {
                pthread_cond_signal(&mon->left_crossed_breadge);
            }
        }

    }
    else if (dir == kLeftDir) {
        pthread_cond_signal(&mon->walk_right); //signal condition for left side shepherds
        pthread_cond_signal(&mon->walk_left); //signal condition for right side shepherds
    }

    mon->bridge_is_busy = false;
}

void* left_shepherd(void* arg)
{
    int num = ((Shepherd*)arg)->num;
    BridgeMonitor* mon = ((Shepherd*)arg)->monitor;

    printf("shepherd #%d: came\n", num);

    // 0. increment queue counter
    mon->left_queue_count++;
    bool left_hat = false;

    pthread_mutex_lock(&mon->mutex); //lock mutex

    if (mon->left_queue_count < 2) {
        alone_cross_bridge_and_leave_hat(mon, num, &left_hat);
        pthread_cond_broadcast(&mon->ready_to_walk_right); //signal to all waiting shepherds
    }
    else {
        while (!mon->hat_on_right) { //until first shepherd took the hat, you can become ready to cross
            pthread_cond_wait(&mon->ready_to_walk_right, &mon->mutex);
        }
    }

    // 2. wait on bridge entry
    while (mon->bridge_is_busy) {
        pthread_cond_wait(&mon->walk_right, &mon->mutex); //join queue on condition variable
    }

    herd_cross_bridge(mon, num, kRightDir, left_hat); //crit section
    
    pthread_mutex_unlock(&mon->mutex); //unlock mutex

    return NULL;
}

void* right_shepherd(void* arg)
{
    int num = ((Shepherd*)arg)->num;
    BridgeMonitor* mon = ((Shepherd*)arg)->monitor;

    printf("shepherd #%d: came\n", num);

    pthread_mutex_lock(&mon->mutex); //lock mutex
    while (mon->bridge_is_busy || mon->hat_on_right) { //wait till no hat & bridge is empty 
        pthread_cond_wait(&mon->walk_left, &mon->mutex); //join queue on condition variable
    }

    herd_cross_bridge(mon, num, kLeftDir, false); //crit section

    pthread_mutex_unlock(&mon->mutex); //unlock mutex

    return NULL;
}

int main()
{
    //set threads
    pthread_t t_arr[10] = {0};
    BridgeMonitor monitor = {.left_queue_count = 0};
    Shepherd shepherd_arr[10];
    for (int i = 0; i < 10; i++) { 
        shepherd_arr[i].monitor = &monitor;
        shepherd_arr[i].num = i;
    }

    //start threads
    for (int i = 0; i < 4; i++) {
        shepherd_arr[i].num = i;
        pthread_create(&(t_arr[i]), NULL, left_shepherd, (void*)(&shepherd_arr[i]));
        usleep(50);
    }
    for (int i = 4; i < 7; i++) {
        shepherd_arr[i].num = i;
        pthread_create(&(t_arr[i]), NULL, right_shepherd, (void*)(&shepherd_arr[i]));
        usleep(50);
    }
    for (int i = 7; i < 10; i++) {
        shepherd_arr[i].num = i;
        pthread_create(&(t_arr[i]), NULL, left_shepherd, (void*)(&shepherd_arr[i]));
        usleep(50);
    }

    //join threads
    for (int i = 0; i < 10; i++) {
        pthread_join(t_arr[i], NULL);
    }

    return 0;
}

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "fifo/fifo.h"

#define kTimeToCrossAlone 10 //usec
#define kTimeToCrossWithHerd 30 //usec
#define kQueueTimeDelta 100 //usec

#define kRightDir 1
#define kLeftDir 2

typedef struct {
    pthread_mutex_t mutex;

    pthread_cond_t walk_right; //availability of walking right direction 
    pthread_cond_t walk_left; //availability of walking left direction
    pthread_cond_t left_came;

    bool chain_walk_right; //right shepherd passes all left shepherds after one, who leave hat
    bool bridge_is_busy;
    bool hat_on_right;

    FIFO left_queue;
    FIFO right_queue;
} BridgeMonitor;

typedef struct {
    int num;
    BridgeMonitor* monitor;
} Shepherd;

//------------ Monitor functions ----------------
bool able_to_leave_hat(BridgeMonitor* mon, int num)
{
    return (mon->left_queue.count == 1);
}

//TODO: monitor or usual function ?
void cross_alone(BridgeMonitor* mon, int num, int dir)
{
    printf("#%d: crossing (%s)\n", num, dir == kRightDir ? "=>" : "<=");
    usleep(kTimeToCrossAlone);
}

void leave_hat(BridgeMonitor* mon, int num)
{
    mon->hat_on_right = true;
    printf("#%d: put down my hat\n", num);
}

void take_hat(BridgeMonitor* mon, int num) 
{
    mon->hat_on_right = false;
    printf("#%d: took my hat\n", num);
}

void imitate_walk_with_herd(BridgeMonitor* mon, int num, int dir)
{
    mon->bridge_is_busy = true;
    printf("#%d + herd: crossing (%s)\n", num, (dir == kRightDir) ? "=>" : "<=");
    usleep(kTimeToCrossWithHerd);
    printf("#%d + herd: finished\n", num);
    mon->bridge_is_busy = false;
}

void cross_with_herd_on_right(BridgeMonitor* mon, int num, Node* self_ptr, bool have_to_leave_hat)
{
    if (have_to_leave_hat) { //we are first
        imitate_walk_with_herd(mon, num, kRightDir);
        take_hat(mon, num);

        // int res;
        // fifo_dequeue(&mon->left_queue, &res); //leave queue //нельзя выписывать себя до того, как пройдет возможность добавиться новым, 
        //                                                     //иначе новый подумает, что он первый и будет оставлять шапку(лишнее)

        // pthread_cond_broadcast(&mon->walk_right); //awake all, but only one enters bridge 
    }
    else { //we are not first
        while (mon->bridge_is_busy && (self_ptr != mon->left_queue.head)) {
            pthread_cond_wait(&mon->walk_right, &mon->mutex);
        }

        imitate_walk_with_herd(mon, num, kRightDir);
    }

    //wait a bit(to decide, to continue queue or not)
    mon->chain_walk_right = true; //to prevent right shepherds from entering bridge, while mutex is free && no hat && bridge is free 
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += kQueueTimeDelta * 1e3;
    pthread_cond_timedwait(&mon->left_came, &mon->mutex, &ts);

    int res;
    fifo_dequeue(&mon->left_queue, &res); //leave queue
    
    //decide, who to signal  
    if (mon->left_queue.count > 0) { //there are people in left queue
        mon->chain_walk_right = true;
        printf("#%d: left_queue = %d\n", num, mon->left_queue.count);
        pthread_cond_broadcast(&mon->walk_right);
    }
    else if (mon->left_queue.count == 0) { //you are the last from left side
        mon->chain_walk_right = false;
        printf("#%d: signaling to right shepherds\n", num);
        pthread_cond_broadcast(&mon->walk_left);
    }
    else { //error
        printf("some stramge problem");
        exit(EXIT_FAILURE);
    }
    fflush(stdout);
}   

void try_to_cross_left(BridgeMonitor* mon, int num)
{
    while (mon->bridge_is_busy || mon->hat_on_right || mon->chain_walk_right) {
        pthread_cond_wait(&mon->walk_left, &mon->mutex);
    }

    imitate_walk_with_herd(mon, num, kLeftDir);

    pthread_cond_signal(&mon->walk_left); //right shepherds signal only to right shepherds
}

//-----------------------------------------------

void* left_shepherd(void* arg)
{
    int num = ((Shepherd*)arg)->num;
    BridgeMonitor* mon = ((Shepherd*)arg)->monitor;
    printf("#%d: came\n", num);    

    pthread_mutex_lock(&mon->mutex);

    //join queue
    Node* self_ptr = fifo_enqueue(&mon->left_queue, num); //join the end of left queue
    bool have_to_leave_hat = able_to_leave_hat(mon, num);
    pthread_cond_signal(&mon->left_came);

    //leave hat, if you are first
    if (have_to_leave_hat) {
        cross_alone(mon, num, kRightDir);
        leave_hat(mon, num);
        cross_alone(mon, num, kLeftDir);
    }
    cross_with_herd_on_right(mon, num, self_ptr, have_to_leave_hat);

    pthread_mutex_unlock(&mon->mutex);

    return NULL;
}

void* right_shepherd(void* arg)
{
    int num = ((Shepherd*)arg)->num;
    BridgeMonitor* mon = ((Shepherd*)arg)->monitor;
    printf("#%d: came\n", num);

    pthread_mutex_lock(&mon->mutex);
    fifo_enqueue(&mon->right_queue, num);

    try_to_cross_left(mon, num);

    pthread_mutex_unlock(&mon->mutex);

    return NULL;
}

void monitor_init(BridgeMonitor* mon)
{
    pthread_mutex_init(&mon->mutex, NULL);

    pthread_cond_init(&mon->walk_left, NULL);
    pthread_cond_init(&mon->walk_right, NULL);
    pthread_cond_init(&mon->left_came, NULL);

    mon->hat_on_right = false;
    mon->bridge_is_busy = false;
    mon->chain_walk_right = false;

    fifo_init(&mon->left_queue);
    fifo_init(&mon->right_queue);
}

int main()
{
    //set threads
    pthread_t t_arr[10] = {0};
    BridgeMonitor monitor;
    monitor_init(&monitor);

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

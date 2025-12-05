#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define kTimeToCrossAlone 100 //usec
#define kTimeToCrossWithHerd 300 //usec

#define kRightDir 1
#define kLeftDir 2

#define kQueueSize 128

typedef struct {
    pthread_mutex_t mutex;

    pthread_cond_t walk_right; //availability of walking right direction 
    pthread_cond_t walk_left; //availability of walking left direction

    bool bridge_is_busy;
    bool hat_on_right;

    int left_queue[kQueueSize];
    int left_queue_counter;

    int right_queue[kQueueSize];
    int right_queue_counter;

} BridgeMonitor;

typedef struct {
    int num;
    BridgeMonitor* monitor;
} Shepherd;

//------------ Monitor functions ----------------

void add_to_left_queue(BridgeMonitor* mon, int num)
{
    pthread_mutex_lock(&mon->mutex);

    if (mon->left_queue_counter < kQueueSize) {
        mon->left_queue[mon->left_queue_counter] = num;
        mon->left_queue_counter++;
    }
    else {
        printf("queue overflow\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&mon->mutex);
}

void add_to_right_queue(BridgeMonitor* mon, int num)
{
    pthread_mutex_lock(&mon->mutex);

    if (mon->right_queue_counter < kQueueSize) {
        mon->right_queue[mon->right_queue_counter] = num;
        mon->right_queue_counter++;
    }
    else {
        printf("queue overflow\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&mon->mutex);
}

bool able_to_leave_hat(BridgeMonitor* mon, int num)
{
    pthread_mutex_lock(&mon->mutex);

    int* ptr = mon->left_queue;
    for (int i = 0; i < mon->left_queue_counter; i++) {
            
        ptr += i;
        if (*ptr == num) {
            break;
        }
        else {
            ptr = mon->left_queue;
        }
    }

    if (ptr != NULL) {
        int pos = *ptr;
        if (pos == 0) {
            pthread_mutex_unlock(&mon->mutex);
            return true;
        } 
        else {
            pthread_mutex_unlock(&mon->mutex); 
            return false;
        }
    }
    else {
        printf("error");
        exit(EXIT_FAILURE);
    }
}

//TODO: monitor or usual function ?
void cross_alone(BridgeMonitor* mon, int num, int dir)
{
    // mon->shepherd_crossing_alone = true;
    printf("shepherd #%d: crossing (%s)\n", num, dir == kRightDir ? "=>" : "<=");
    usleep(kTimeToCrossAlone);

}

void leave_hat(BridgeMonitor* mon, int num)
{
    pthread_mutex_lock(&mon->mutex);

    mon->hat_on_right = true;
    printf("shepherd #%d: put down my hat\n", num);

    pthread_mutex_unlock(&mon->mutex);
}

void take_hat(BridgeMonitor* mon, int num) 
{
    pthread_mutex_lock(&mon->mutex);

    mon->hat_on_right = false;
    printf("shepherd #%d: took my hat\n", num);

    pthread_mutex_unlock(&mon->mutex);
}

void imitate_walk_with_herd(BridgeMonitor* mon, int num, int dir)
{
    mon->bridge_is_busy = true;
    printf("shepherd + herd #%d: crossing (%s)\n", num, (dir == kRightDir) ? "=>" : "<=");
    usleep(kTimeToCrossWithHerd);
    printf("shepherd + herd #%d: finished\n", num);
    mon->bridge_is_busy = false;
}

void cross_with_herd_on_right(BridgeMonitor* mon, int num, bool i_am_first)
{
    pthread_mutex_lock(&mon->mutex);

    if (i_am_first) {
        imitate_walk_with_herd(mon, num, kRightDir);
        take_hat(mon, num);
        mon->left_queue_counter--;

        pthread_cond_signal(&mon->walk_right);
    }   
    else {
        while (mon->bridge_is_busy) {
            pthread_cond_wait(&mon->walk_right, &mon->mutex);
        }

        imitate_walk_with_herd(mon, num, kRightDir);
        mon->left_queue_counter--;

        //decide, who to signal  
        if (mon->left_queue_counter > 0) { //there are people in left queue
            pthread_cond_signal(&mon->walk_right);
        }
        else if (mon->left_queue_counter == 0) { //you are the last from left side
            pthread_cond_signal(&mon->walk_left);
        }
        else { //error
            printf("some stramge problem");
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutex_unlock(&mon->mutex);
}

void try_to_cross_left(BridgeMonitor* mon, int num)
{
    pthread_mutex_lock(&mon->mutex);

    while (mon->bridge_is_busy) {
        pthread_cond_wait(&mon->walk_left, &mon->mutex);
    }

    imitate_walk_with_herd(mon, num, kLeftDir);

    pthread_cond_signal(&mon->walk_left); //right shepherds signal only to right shepherds

    pthread_mutex_unlock(&mon->mutex);
}

//-----------------------------------------------

void* left_shepherd(void* arg)
{
    int num = ((Shepherd*)arg)->num;
    BridgeMonitor* mon = ((Shepherd*)arg)->monitor;
    printf("shepherd #%d: came\n", num);    

    //join queue
    add_to_left_queue(mon, num);

    //leave hat, if you are first
    if (able_to_leave_hat(mon, num)) {
        cross_alone(mon, num, kRightDir);
        leave_hat(mon, num);
        cross_alone(mon, num, kLeftDir);
        cross_with_herd_on_right(mon, num, true);
    }
    else {
        cross_with_herd_on_right(mon, num, false);
    }   

    return NULL;
}

void* right_shepherd(void* arg)
{
    int num = ((Shepherd*)arg)->num;
    BridgeMonitor* mon = ((Shepherd*)arg)->monitor;
    printf("shepherd #%d: came\n", num);

    try_to_cross_left(mon, num);

    return NULL;
}

int main()
{
    //set threads
    pthread_t t_arr[10] = {0};
    BridgeMonitor monitor = {0};
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

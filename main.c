#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Направления
#define LEFT_TO_RIGHT 0
#define RIGHT_TO_LEFT 1

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;       // условие для ожидания
    int hat;                   // 0 = нет шапки, 1 = шапка стоит
    int direction;             // текущее направление движения (если шапка стоит)
    int waiting_count;         // количество пастухов, ждущих в очереди (в том же направлении)
} MonitorBridge;

MonitorBridge bridge = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
    .hat = 0,
    .direction = -1, //no direction
    .waiting_count = 0 //amount of people in queue of active direction
                       //helps to continue crossing in this direction 
};

typedef struct {
    int dir; //direction of shepherd
    int num; //number of sheperd
} ShepherdInfo;

void shepherd(ShepherdInfo* info);

void* shepherd_thread(void* arg) {
    ShepherdInfo* info = (ShepherdInfo*)arg;
    shepherd(info);
    return NULL;
}

int main() {
    pthread_t t_arr[10] = {0};

    //set shepherd directions
    ShepherdInfo shepherd_arr[10] = {{.dir = LEFT_TO_RIGHT}, 
                                     {.dir = LEFT_TO_RIGHT},
                                     {.dir = LEFT_TO_RIGHT},
                                     {.dir = LEFT_TO_RIGHT},
                                     
                                     {.dir = RIGHT_TO_LEFT},
                                     {.dir = RIGHT_TO_LEFT},
                                     {.dir = RIGHT_TO_LEFT},

                                     {.dir = LEFT_TO_RIGHT},
                                     {.dir = LEFT_TO_RIGHT},
                                     {.dir = LEFT_TO_RIGHT}};

    //set shepherd numbers (for illustrating only)
    for (int i = 0; i < 10; i++) {
        shepherd_arr[i].num = i;
        pthread_create(&(t_arr[i]), NULL, shepherd_thread, (void*)(&shepherd_arr[i]));
        usleep(100);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(t_arr[i], NULL);
    }

    return 0;
}

void shepherd(ShepherdInfo* info) {
    int dir = info->dir; //self direstion
    int num = info->num; //self num (only for printf)

    int my_hat = 1; //shows, is there a hat on shepherd 

    pthread_mutex_lock(&bridge.mutex); //lock mutex
    // if hat on bridge AND opposite direction moving
    while (bridge.hat && bridge.direction != dir) {
        printf("shepherd %d wait: opposite direction moving (%s)\n",
               num, (dir == LEFT_TO_RIGHT) ? "=>" : "<=");
        pthread_cond_wait(&bridge.cond, &bridge.mutex); //waiting
    }

    // waited for other direction to cross bridge  
    // no hat => begin crossing the bridge
    if (!bridge.hat) {
        bridge.hat = 1;
        bridge.direction = dir;
        bridge.waiting_count = 0;
        printf("shepherd %d puts hat, begins moving (%s)\n",
               num, (dir == LEFT_TO_RIGHT) ? "=>" : "<=");
    } 
    else { // there is hat and already moving in this direction -- join column
        bridge.waiting_count++;
        printf("shepherd %d joins collumn (%s), herd in collumn: %d\n",
               num, (dir == LEFT_TO_RIGHT) ? "=>" : "<=", bridge.waiting_count);
    }
    pthread_mutex_unlock(&bridge.mutex); //unlock mutex

    //===== Critical section: crossing the bridge =====
    printf("shepherd %d crosses the bridge (%s)\n", num, (dir == LEFT_TO_RIGHT) ? "=>" : "<=");
    sleep(1); //imitating crossing time
    //===== End of critical section =====

    pthread_mutex_lock(&bridge.mutex);
    // decrement amount of people in queue 
    if (bridge.waiting_count > 0) {
        bridge.waiting_count--;

        // If there are people in queue -> not awakening waiting processes -> moving in this direction continues
        if (bridge.waiting_count > 0) {
            bridge.hat = 0; //take our hat
            printf("shepherd %d finished crossing and took hat (still %d in queue)\n",
            num, bridge.waiting_count);
        } 
        else {
            // If we are last in queue -> take hat, free bridge
            bridge.hat = 0;
            bridge.direction = -1;
            printf("shepherd %d took hat, bridge is free\n", dir);
            pthread_cond_broadcast(&bridge.cond); //awake all shepherds
        }
    }
    else {
        // If we are the one crossing in this dir -> take hat, free bridge
        bridge.hat = 0;
        bridge.direction = -1;
        printf("shepherd %d took hat, bridge is free\n", num);
        pthread_cond_broadcast(&bridge.cond);
    }
    pthread_mutex_unlock(&bridge.mutex);
}

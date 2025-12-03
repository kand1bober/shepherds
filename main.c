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

void shepherd_cross_bridge(ShepherdInfo* info);

void* shepherd_thread(void* arg) {
    ShepherdInfo* info = (ShepherdInfo*)arg;
    shepherd_cross_bridge(info);
    return NULL;
}

int main() {
    pthread_t t1, t2, t3, t4;

    ShepherdInfo shepherd_1 = {.num = 1, .dir = LEFT_TO_RIGHT};
    ShepherdInfo shepherd_2 = {.num = 2, .dir = LEFT_TO_RIGHT};
    ShepherdInfo shepherd_3 = {.num = 3, .dir = RIGHT_TO_LEFT};
    ShepherdInfo shepherd_4 = {.num = 4, .dir = LEFT_TO_RIGHT};

    pthread_create(&t1, NULL, shepherd_thread, (void*)&shepherd_1);
    // sleep(1); // чтобы показать очередь
    pthread_create(&t2, NULL, shepherd_thread, (void*)&shepherd_2);
    // sleep(1);
    pthread_create(&t3, NULL, shepherd_thread, (void*)&shepherd_3); 
    // sleep(1);
    pthread_create(&t4, NULL, shepherd_thread, (void*)&shepherd_4);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    return 0;
}

void shepherd_cross_bridge(ShepherdInfo* info) {
    int dir = info->dir; //self direstion
    int num = info->num; //self num (only for printf)

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

        // Если ещё есть кто-то в очереди — оставляем шапку, надо продолжить движение колонны в этом направлениии
        if (bridge.waiting_count > 0) {
            bridge.hat = 0;
            printf("shepherd %d finished crossing and took hat (still %d in queue)\n",
            num, bridge.waiting_count);
        } else {
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

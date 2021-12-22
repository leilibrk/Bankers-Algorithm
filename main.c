#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUMBER_OF_RESOURCES 5
#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_DEMANDS 3
sem_t mutex;
int available[NUMBER_OF_RESOURCES];
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int need [NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];


bool satisfyNeeds(int i, int work[NUMBER_OF_RESOURCES]) {
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        if(need[i][j] > work[j])
            return false;
    }
    return true;
}

bool allAreFinished(bool finish[NUMBER_OF_CUSTOMERS]) {
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        if(!finish[i])
            return false;
    }
    return true;
}

bool isSafe(){
    int work[NUMBER_OF_RESOURCES];
    bool finish[NUMBER_OF_CUSTOMERS];
    for(int i = 0; i < NUMBER_OF_RESOURCES; i++){
        work[i] = available[i];
    }
    for(int i = 0; i < NUMBER_OF_CUSTOMERS; i++){
        finish[i] = false;
    }
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        if(!finish[i] && satisfyNeeds(i, work)){
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                work[j] = work[j] + allocation[i][j];
                finish[i] = true;
            }
        }
    }
    if(allAreFinished(finish))
        return true;

    return false;

}

bool legalRequest(int customer_num, int request[NUMBER_OF_RESOURCES]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if(request[i] > need[customer_num][i])
            return false;
    }
    return true;
}

bool availableResources(int request[NUMBER_OF_RESOURCES]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if(request[i] > available[i])
            return false;
    }
    return true;
}
void request_print(int req[NUMBER_OF_RESOURCES]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        printf("%d", req[i]);
    }
    printf("\n");
}

bool requestIsZero(int req[NUMBER_OF_RESOURCES]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (req[i] != 0)
            return false;
    }
    return true;
}

int Release_Resources(int customer_num, int request[NUMBER_OF_RESOURCES]) {
    sem_wait(&mutex);
    if(requestIsZero(request)){
        sem_post(&mutex);
        return 0;
    }
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if(request[i] > allocation[customer_num][i]){
            printf("customer %d wanted to release its resources; the released recourses are: ", customer_num);
            request_print(request);
            printf("Release request refused! more than allocation!\n");
            sem_post(&mutex);
            return -1;
        }
    }
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] = available[i] + request[i];
        allocation[customer_num][i] = allocation[customer_num][i] - request[i];
        need[customer_num][i] = need[customer_num][i] + request[i];
    }
    printf("customer %d released its resources; the released recourses are: ", customer_num);
    request_print(request);
    sem_post(&mutex);
    return 0;
}

int Request_Resources(int customer_num, int request[NUMBER_OF_RESOURCES]){
    sem_wait(&mutex);
    if(requestIsZero(request)){
        sem_post(&mutex);
        return 0;
    }
    printf("customer %d requests for resources, the request is: ", customer_num);
    request_print(request);
    if(!legalRequest(customer_num, request)){
        printf("Request refused! Customer %d has exceeded its maximum claim!\n", customer_num);
        sem_post(&mutex);
        return -1;
    }
    if(!availableResources(request)){
        printf("Request refused! Resources not enough for customer %d! It should wait!\n", customer_num);
        sem_post(&mutex);
        return -1;
    }
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] = available[i] - request[i];
        allocation[customer_num][i] = allocation[customer_num][i] + request[i];
        need[customer_num][i] = need[customer_num][i] - request[i];
    }

    if(isSafe()) {
        sem_post(&mutex);
        printf("Request accepted!\n");
        return 0;
    }
    else{
        printf("Request refused! By this allocation to customer %d, system will be unsafe!\n", customer_num);
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            available[i] = available[i] + request[i];
            allocation[customer_num][i] = allocation[customer_num][i] - request[i];
            need[customer_num][i] = need[customer_num][i] + request[i];
        }
        sem_post(&mutex);
        return -1;
    }
}
void *runner(int customer_num){
    int req[NUMBER_OF_RESOURCES];
    srand(time(NULL));
    int r;
    for (int i = 0; i < NUMBER_OF_DEMANDS; i++) {
        r = rand() % 3;
        if(r == 0){
            //release
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                req[j] = rand() % (allocation[customer_num][j] + 1);
            }
            Release_Resources(customer_num, req);
        }
        else{
            //request
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                req[j] = rand() % (need[customer_num][j] + 1);
            }
            Request_Resources(customer_num, req);
        }
    }

}
int main(int argc, char** argv) {
    if(argc < NUMBER_OF_RESOURCES){
        printf("Not enough arguments\n");
        return -1;
    }
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] = strtol(argv[i+1], NULL, 10);
    }
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        printf("av[%d]:%d\n",i,available[i]);
    }
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            allocation[i][j] = 0;
        }
    }
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        printf("How much customer %d wants from all resources?\n", i);
        scanf("%d %d %d %d %d", &maximum[i][0], &maximum[i][1], &maximum[i][2], &maximum[i][3], &maximum[i][4]);
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            need[i][j] = maximum[i][j];
        }
    }
    sem_init(&mutex, 0, 1);
    pthread_t customers[NUMBER_OF_CUSTOMERS];
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_create(&customers[i], NULL, (void *) runner, (void *)(intptr_t) i);
    }
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }
    return 0;
}

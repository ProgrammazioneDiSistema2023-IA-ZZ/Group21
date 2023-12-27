#ifndef BANKER_H
#define BANKER_H

#include <pthread.h>

#define NUM_THREADS 5
#define NUM_RESOURCES 3

extern pthread_mutex_t mutex;
extern pthread_cond_t condition;
extern int available[NUM_RESOURCES];
extern int maximum[NUM_THREADS][NUM_RESOURCES];
extern int allocation[NUM_THREADS][NUM_RESOURCES];
extern int need[NUM_THREADS][NUM_RESOURCES];

void calculate_need();
int request_resources(int thread_id, int request[]);
void release_resources(int thread_id, int release[]);
int is_safe();
void* thread_function(void* arg);
void initialize_resources();

#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "banker.h"

// Definizione del numero di thread e delle risorse disponibili
#define NUM_THREADS 5
#define NUM_RESOURCES 3

// Dichiarazione delle variabili per la sincronizzazione e lo stato delle risorse
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
int available[NUM_RESOURCES] = {3, 3, 2};
int maximum[NUM_THREADS][NUM_RESOURCES] = {
    {7, 5, 3},
    {3, 2, 2},
    {9, 0, 2},
    {2, 2, 2},
    {4, 3, 3}
};
int allocation[NUM_THREADS][NUM_RESOURCES] = {0};
int need[NUM_THREADS][NUM_RESOURCES];

// Funzione per calcolare le risorse necessarie per ciascun thread
void calculate_need() {
    for (int i = 0; i < NUM_THREADS; ++i) {
        for (int j = 0; j < NUM_RESOURCES; ++j) {
            need[i][j] = maximum[i][j] - allocation[i][j];
        }
    }
}

// Funzione per richiedere risorse
int request_resources(int thread_id, int request[]) {
    pthread_mutex_lock(&mutex);

    // Verifica se la richiesta è entro i limiti massimi
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        if (request[i] > need[thread_id][i]) {
            pthread_mutex_unlock(&mutex);
            return -1;  // La richiesta supera il limite massimo
        }
    }

    // Verifica se la richiesta può essere concessa immediatamente
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        if (request[i] > available[i]) {
            pthread_mutex_unlock(&mutex);
            return 0;  // La richiesta non può essere concessa immediatamente
        }
    }

    // Simulazione dell'allocazione e verifica della sicurezza del sistema
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        available[i] -= request[i];
        allocation[thread_id][i] += request[i];
        need[thread_id][i] -= request[i];
    }

    // Verifica se il sistema è in uno stato sicuro
    if (!is_safe()) {
        // Rollback dell'allocazione in caso di insicurezza
        for (int i = 0; i < NUM_RESOURCES; ++i) {
            available[i] += request[i];
            allocation[thread_id][i] -= request[i];
            need[thread_id][i] += request[i];
        }

        pthread_mutex_unlock(&mutex);
        return 0;  // Il sistema non è in uno stato sicuro
    }

    pthread_mutex_unlock(&mutex);
    return 1;  // Richiesta concessa
}



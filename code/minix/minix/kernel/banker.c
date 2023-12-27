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

// Funzione per rilasciare risorse
void release_resources(int thread_id, int release[]) {
    pthread_mutex_lock(&mutex);

    // Rilascio delle risorse
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        available[i] += release[i];
        allocation[thread_id][i] -= release[i];
        need[thread_id][i] += release[i];
    }

    // Sveglia tutti i thread in attesa
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&mutex);
}

// Funzione per verificare se il sistema è in uno stato sicuro
int is_safe() {
    int work[NUM_RESOURCES];
    int finish[NUM_THREADS] = {0};

    // Inizializzazione del lavoro disponibile alle risorse attuali
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        work[i] = available[i];
    }

    // Algoritmo per verificare la sicurezza
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (!finish[i]) {
            int can_allocate = 1;
            for (int j = 0; j < NUM_RESOURCES; ++j) {
                if (need[i][j] > work[j]) {
                    can_allocate = 0;
                    break;
                }
            }
            // Controllare se tutte le risorse richieste da un thread possono essere allocate in modo che il sistema rimanga in uno stato sicuro.
            if (can_allocate) {
                finish[i] = 1;
                for (int j = 0; j < NUM_RESOURCES; ++j) {
                    work[j] += allocation[i][j];
                }

                i = -1;  // Ricomincia il loop per verificare i processi appena terminati
            }
        }
    }

    // Verifica se tutti i processi sono terminati
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (!finish[i]) {
            return 0;  // Il sistema non è in uno stato sicuro
        }
    }

    return 1;  // Il sistema è in uno stato sicuro
}


void* thread_function(void* arg) {
    int thread_id = *((int*)arg);
    int request[NUM_RESOURCES];

    // Simulazione di richieste e rilasci casuali
    while (1) {
        sleep(1);

        for (int i = 0; i < NUM_RESOURCES; ++i) {
            request[i] = rand() % (maximum[thread_id][i] + 1);
        }

        int request_result = request_resources(thread_id, request);

        if (request_result == 1) {
            printf("Thread %d ha rilasciato risorse: [%d %d %d]\n",
                   thread_id, request[0], request[1], request[2]);
            release_resources(thread_id, request);
        } else if (request_result == -1) {
            printf("Thread %d ha richiesto risorse oltre il limite massimo. Riprova...\n", thread_id);
        } else {
            printf("Thread %d in attesa di risorse. Riprova...\n", thread_id);
        }
    }

    return NULL;
}


int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Calcola le risorse necessarie per ciascun thread
    calculate_need();

    // Creazione dei thread
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, (void*)&thread_ids[i]);
    }

    // Attesa dei thread terminati
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}



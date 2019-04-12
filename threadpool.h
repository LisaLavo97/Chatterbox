/*
 * @file threadpoo.h
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include "queue.h"
#include "user_manager.h"
#include <pthread.h>


/* Threadpool */
typedef struct threadpool {
    pthread_t* threads;
    queue_t fd_queue_1;
    queue_t fd_queue_2;
    user_manager_t* user_manager;
} threadpool_t;


/*
 * @function threadpool_init
 * @brief Crea ed inizializza i thread del pool e le strutture contenute nella struttura threadpool
 * @param num_threads
 * @param function(void * args) funzione che rappresenta il task svolto dai thread del pool
 * @user_manager gestore degli utenti
 * @return il threadpool inizializzato
 */
threadpool_t* threadpool_init (int num_threads, void* function(void* args), user_manager_t* user_manager);


/*
 * @function destroy_pool
 * @brief Distrugge una struttura threadpool
 * @param pool threadpool da distruggere
 * @param num_threads numero di thread nel pool
 * @return 1 in caso non ci siano errori, -1 altrimenti
 */
int destroy_pool (threadpool_t* pool, int num_threads);


#endif /* THREADPOOL_H_ */

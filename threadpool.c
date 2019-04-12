/*
 * @file threadpoo.c
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>


// Inizializzazione threadpool
threadpool_t* threadpool_init (int num_threads, void* function(void * args), user_manager_t* user_manager) {
    if (num_threads < 0){
        num_threads = 0;
    }
    
    // Creo nuovo threadpool
    threadpool_t* pool = (threadpool_t*) malloc(sizeof(threadpool_t));
    if (pool == NULL){
        //err("threadpool_init(): Could not allocate memory for thread pool\n");
        return NULL;
    }
    
    // Creo le code
    create_queue(&pool -> fd_queue_1);
    create_queue(&pool -> fd_queue_2);
    
    pool -> user_manager = user_manager;
    
    // Creo i thread nel pool
    pool -> threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    
    if (pool -> threads == NULL) {
        //err("threadpool_init(): Could not allocate memory for threads\n");
        delete_queue(&(pool -> fd_queue_1));
        delete_queue(&(pool -> fd_queue_2));
        free(pool);
        pool = NULL;
        return NULL;
    }

    //Inizializzazione
    int i, err;
    for (i=0; i < num_threads; i++) {
        err = pthread_create(&pool -> threads[i], NULL, function, pool);
        if(err != 0) {
            perror("pthread_create");
            fprintf(stderr, "Errore nella creazione dei thread nel pool");
            free(pool);
            pool = NULL;
            return NULL;
        }
    }
    return pool;
    
}


int destroy_pool (threadpool_t* pool, int num_threads) {
    
    if(pool == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    int i;
    int tmp = -1;
    for(i=0; i < num_threads; i++)
        enqueue(&pool -> fd_queue_1, tmp);
    
    if(pthread_cond_broadcast(&pool -> fd_queue_1.cond_queue) != 0)
        return -1;
    
    for(i=0; i < num_threads; i++)
        pthread_join(pool -> threads[i], NULL);
    
    delete_queue(&pool -> fd_queue_1);
    delete_queue(&pool -> fd_queue_2);
    
    free(pool->threads);
    
    free(pool);
    pool = NULL;
    
    return 1;
    
}

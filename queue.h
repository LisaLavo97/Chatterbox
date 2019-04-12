/*
 * @file queue.h
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifndef QUEUE_H_
#define QUEUE_H_
 /* coda_h */

typedef struct node {
    void* info;
    struct node* next;
} node_t;


typedef struct queue {
    int dim;
    node_t* head;
    node_t* tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond_queue;
} queue_t;


/*
 * @function create_queue
 * @brief crea una coda
 * @param new coda
 *
 */
void create_queue(queue_t* new);


/*
 * @function delete_queue
 * @brief Elimina la coda passata come argomento
 * @param queue coda da eliminare
 *
 */
void delete_queue(queue_t* queue);


/*
 * @function delete_queue
 * @brief Aggiunge un elemento in testa alla coda
 * @param queue struttura nella quale aggiungere l'elemento
 * @param elem elemento da aggiungere
 * @return 1 in caso di successo, 0 in caso di errore
 */
int enqueue(queue_t* queue, void* elem);


/*
 * @function dequeue_1
 * @brief Estrae l'elemento in fondo alla coda.
 *        Se la coda è vuota, si blocca sulla condition wait
 * @param queue struttura dalla quale estrarre l'elemento in coda
 * @return l'elemento in coda
 */
void* dequeue_1(queue_t* queue);


/*
 * @function dequeue_2
 * @brief Estrae l'elemento in fondo alla coda.
 *        Se la coda è vuota, restituisce NULL
 * @param queue struttura dalla quale estrarre l'elemento in coda
 * @return l'elemento in coda, oppure NULL se la coda è vuota
 */
void* dequeue_2(queue_t* queue);


/*
 * @function delete_elem
 * @brief Elimina l'elemento puntato da elem dalla coda
 * @param queue struttura dalla quale rimuovere l'elemento
 * @return la coda con l'elemento rimosso
 */
queue_t* delete_elem (queue_t* queue, void* elem);


void lock_queue (queue_t* queue);


void unlock_queue (queue_t* queue);


/*
 * @function set_user_offline
 * @brief Elimina l'utente con file descriptor == fd dalla coda degli utenti connessi
 * @param utenti_connessi coda degli utenti online
 * @param fd file descriptor relativo al client che deve essere disconnesso
 */
void set_user_offline (queue_t* utenti_connessi, int fd);


/*
 * @function find_fd
 * @brief Ricerca l'utente con file descriptor == fd nella coda degli utenti connessi
 * @param utenti_connessi coda degli utenti online
 * @param fd file descriptor relativo al client
 * @return la struttura utente trovata, o NULL se l'utente non è presente nella coda
 */
void* find_fd (queue_t* utenti_connessi, int fd);


#endif /* QUEUE_H_ */

/*
 * @file queue.c
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#include "stats.h"
#include "queue.h"
#include "user_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


void lock_queue (queue_t* queue) {
    pthread_mutex_lock(&queue -> mutex);
}


void unlock_queue (queue_t* queue) {
    pthread_mutex_unlock(&queue -> mutex);
}


/* Crea la coda */
void create_queue (queue_t* new) {
    new -> head = NULL;
    new -> tail = NULL;
    new -> dim = 0;
    pthread_mutex_init(&new -> mutex, NULL);
    pthread_cond_init(&new -> cond_queue, NULL);
}


 /* Elimina la coda */
void delete_queue(queue_t* queue) {
    if(queue == NULL)
        return;
    
    node_t* curr = (queue) -> head;
    node_t* succ = NULL;
    
    while (curr != NULL) {
        succ = curr -> next;
        free(curr);
        curr = succ;
    }
}


/* Aggiunge un elemento in testa alla coda */
int enqueue(queue_t* queue, void* elem) {
    if ((queue == NULL) || (elem == NULL)) {
        return 0;
    }
    lock_queue(queue);
    node_t* new_node = malloc(sizeof(node_t));
    if(new_node == NULL){
        unlock_queue(queue);
        return 0;
    }
    new_node -> info = elem;
    new_node -> next = queue -> head;
    queue -> head = new_node;
    
    if(queue -> tail == NULL)
        queue -> tail = queue -> head;

    queue -> dim ++;
    pthread_cond_signal(&queue -> cond_queue);
    unlock_queue(queue);
    return 1;
}


/* Estrae l'elemento in fondo alla coda.
 Se la coda è vuota, si blocca sulla condition wait
 */
void* dequeue_1(queue_t* queue) {
    void* info = NULL;
    lock_queue(queue);
    /* Coda vuota */
    while (queue -> head == NULL)
        pthread_cond_wait(&queue -> cond_queue, &queue -> mutex);
    
    info = queue -> tail -> info;
    
    /* Coda con un solo elemento */
    if (queue -> head == queue -> tail) {
        free(queue -> head);
        queue -> head = NULL;
        queue -> tail = NULL;
        queue -> dim --;
        unlock_queue(queue);
        return info;
    }
    
    node_t* curr = queue -> head;
    while(curr -> next != queue -> tail)
        curr = curr -> next;
    
    queue -> tail = curr;
    
    free(curr -> next);
    curr -> next = NULL;
    queue -> dim --;
    unlock_queue(queue);
    return info;
}


/* Estrae l'elemento in fondo alla coda.
 Se la coda è vuota, restituisce NULL
 */
void* dequeue_2(queue_t* queue) {
    void* info = NULL;
    
    lock_queue(queue);
    
    /* Coda vuota */
    if (queue -> head == NULL) {
        unlock_queue(queue);
        return NULL;
    }
    
    info = queue -> tail -> info;
    
    /* Coda con un solo elemento */
    if (queue -> head == queue -> tail) {
        free(queue -> head);
        queue -> head = NULL;
        queue -> tail = NULL;
        queue -> dim --;
        unlock_queue(queue);
        return info;
    }
    
    node_t* curr = queue -> head;
    while(curr -> next != queue -> tail)
        curr = curr -> next;
    
    queue -> tail = curr;
    
    free(curr -> next);
    curr -> next = NULL;
    queue -> dim --;
    unlock_queue(queue);
    return info;
}


queue_t* delete_elem (queue_t* queue, void* elem) {
    lock_queue(queue);
    
    // Coda vuota
    if (queue -> head == NULL) {
        unlock_queue(queue);
        return queue;
    }
    
    // Coda con un solo elemento
    if (queue -> head == queue -> tail) {
        if (queue -> head -> info == elem) {
            free(queue -> head);
            queue -> head = NULL;
            queue -> tail = NULL;
            queue -> dim  --;
            unlock_queue(queue);
            return queue;
        }
    }
    
    node_t* prec = NULL;
    node_t* curr = queue -> head;
    
    while(curr -> info != elem && curr != NULL) {
        prec = curr;
        curr = curr -> next;
    }
    
    if(curr == NULL) {
        unlock_queue(queue);
        return NULL;
    }
    
    if(queue -> head == curr) {
        queue -> head = curr -> next;
        free(curr);
        queue -> dim --;
        unlock_queue(queue);
        return queue;
    }
    
    if(queue -> tail == curr) {
        queue -> tail = prec;
        free(curr);
        queue -> dim --;
        unlock_queue(queue);
        return queue;
    }
    
    prec -> next = curr -> next;
    free(curr);
    queue -> dim --;
    
    unlock_queue(queue);
    return queue;
}


void set_user_offline (queue_t* utenti_connessi, int fd) {
    lock_queue(utenti_connessi);
    
    node_t* prec = NULL;
    node_t* curr = utenti_connessi -> head;
    
    if(curr == NULL) {
        unlock_queue(utenti_connessi);
        return;
    }
    utente_t* user = NULL;
    user = (utente_t*) curr -> info;
    
    while(curr != NULL && user -> fd != fd) {
        prec = curr;
        curr = curr -> next;
        if(curr != NULL)
            user = (utente_t*) curr -> info;
    }
    
    if(curr == NULL) {
        unlock_queue(utenti_connessi);
        return;
    }
    
    extern int client_connessi;
    extern pthread_mutex_t mutex_connessi;
    
    update_statistics(0, -1, 0, 0, 0, 0, 0);
    
    // if(user -> fd == fd)
    user -> fd = -1;
    user -> isOnline = 0;
    if(prec == NULL)
        utenti_connessi -> head = curr -> next;
    else
        prec -> next = curr -> next;
    
    free(curr);
    utenti_connessi -> dim --;
    unlock_queue(utenti_connessi);
}


void* find_fd (queue_t* utenti_connessi, int fd) {
    lock_queue(utenti_connessi);
    
    node_t* prec = NULL;
    node_t* curr = utenti_connessi -> head;
    
    if(curr == NULL) {
        unlock_queue(utenti_connessi);
        return NULL;
    }
    utente_t* user = NULL;
    user = (utente_t*) curr -> info;
    
    while(curr != NULL && user -> fd != fd) {
        prec = curr;
        curr = curr -> next;
        if(curr != NULL)
            user = (utente_t*) curr -> info;
    }
    
    if(curr == NULL) {
        unlock_queue(utenti_connessi);
        return NULL;
    }
    
    // user -> fd == fd
    unlock_queue(utenti_connessi);
    return user;
    
}

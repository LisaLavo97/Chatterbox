//
//  abs_hashtable.h
//


#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "user.h"
#include "message.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

//Hashtable element structure
typedef struct hash_elem {
    struct hash_elem* next; // Elemento succ in caso di collisione
    void* data;    // Puntatore all'elemento
    void* key;      // Chiave dell'elemento
} hash_elem_t;


//Hashtabe structure
typedef struct hashtable {
    int nbuckets;
    hash_elem_t** buckets;
    unsigned int (*hash_function)(void*);
    int (*hash_key_compare)(void*, void*);
    pthread_mutex_t locks[8];
} hashtable_t;


unsigned int hash_pjw(void* key);


int string_compare(void* a, void* b);


unsigned int lock_hash (hashtable_t* ht, void* key);


void unlock_hash (hashtable_t* ht, unsigned int hash_val);


hashtable_t* icl_create_hashtable(int nbuckets);


void* icl_hash_find(hashtable_t*, void*);


hash_elem_t* icl_hash_insert(hashtable_t*, void*, void *);


int icl_hash_destroy(hashtable_t*, void (*)(void*), void (*)(void*)), icl_hash_dump(FILE *, hashtable_t *);


int icl_hash_delete(hashtable_t *ht, void* key, void (*free_key)(void*), void (*free_data)(void*) );

/*
* @function post_txt_all
* @brief Funzione per l'invio di un messaggio testuale a tutti gli utenti registrati
* @param ht hashtable contenente tutti gli utenti registrati
* @param nbuckets numero di buckets dell'hashtable
* @param sender nickname dell'utente mittente
* @param msg messaggio da inviare
*/
void post_txt_all(hashtable_t *ht, int nbuckets, char* sender, message_t msg);


#endif /* HASHTABLE_H_ */

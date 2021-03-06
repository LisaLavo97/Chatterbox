//
//  abs_hashtable.c
//
/**
 * @author Jakub Kurzak */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include "hashtable.h"
#include "connections.h"
#include "configuration.h"
#include "stats.h"

#define BITS_IN_int     ( sizeof(int) * CHAR_BIT )
#define THREE_QUARTERS  ((int) ((BITS_IN_int * 3) / 4))
#define ONE_EIGHTH      ((int) (BITS_IN_int / 8))
#define HIGH_BITS       ( ~((unsigned int)(~0) >> ONE_EIGHTH ))
/**
 * A simple string hash.
 *
 * An adaptation of Peter Weinberger's (PJW) generic hashing
 * algorithm based on Allen Holub's version. Accepts a pointer
 * to a datum to be hashed and returns an unsigned integer.
 * From: Keith Seymour's proxy library code
 *
 * @param[in] key -- the string to be hashed
 *
 * @returns the hash index
 */
 unsigned int hash_pjw(void* key) {
    char *datum = (char *)key;
    unsigned int hash_value, i;
    
    if(!datum) return 0;
    
    for (hash_value = 0; *datum; ++datum) {
        hash_value = (hash_value << ONE_EIGHTH) + *datum;
        if ((i = hash_value & HIGH_BITS) != 0)
            hash_value = (hash_value ^ (i >> THREE_QUARTERS)) & ~HIGH_BITS;
    }
    return (hash_value);
}


unsigned int string_hash_function (void* key) {
    unsigned int hash = 5381;
    int c;
    unsigned char *str = (unsigned char *) key;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; 
    
    return hash;
}


 int string_compare(void* a, void* b) {
    return (strcmp( (char*)a, (char*)b ) == 0);
}


unsigned int lock_hash (hashtable_t* ht, void* key) {
    unsigned int hash_val = (* ht->hash_function)(key) % ht->nbuckets;
    pthread_mutex_lock(&ht->locks[hash_val % 8]);
    return hash_val;
}


void unlock_hash (hashtable_t* ht, unsigned int hash_val) {
    pthread_mutex_unlock(&ht->locks[hash_val % 8]);
}


/**
 * Create a new hash table.
 *
 * @param[in] nbuckets -- number of buckets to create
 * @returns pointer to new hash table.
 */

hashtable_t* icl_create_hashtable(int nbuckets) {
    hashtable_t* ht;
    int i;
    
    ht = (hashtable_t*) malloc(sizeof(hashtable_t));
    if(!ht) return NULL;
    
    ht->buckets = (hash_elem_t**) malloc(nbuckets * sizeof(hash_elem_t*));
    if(!ht->buckets)
        return NULL;
    
    ht->nbuckets = nbuckets;
    for(i=0; i < ht->nbuckets; i++)
        ht->buckets[i] = NULL;
    
    for(i=0; i < 8; i++)
        pthread_mutex_init(&ht->locks[i], NULL);
    
    ht->hash_function = string_hash_function;
    ht->hash_key_compare = string_compare;
    
    
    return ht;
}


/**
 * Search for an entry in a hash table.
 *
 * @param ht -- the hash table to be searched
 * @param key -- the key of the item to search for
 *
 * @returns pointer to the data corresponding to the key.
 *   If the key was not found, returns NULL.
 */

void* icl_hash_find(hashtable_t *ht, void* key) {
    hash_elem_t* curr;
    unsigned int hash_val;
    
    if(!ht || !key)
        return NULL;
    
    hash_val = (* ht->hash_function)(key) % ht->nbuckets;
    for (curr=ht->buckets[hash_val]; curr != NULL; curr=curr->next) {
        if ( ht->hash_key_compare(curr->key, key)) {
            return(curr->data);
        }
    }
    return NULL;
}


/**
 * Insert an item into the hash table.
 *
 * @param ht -- the hash table
 * @param key -- the key of the new item
 * @param data -- pointer to the new item's data
 *
 * @returns pointer to the new item.  Returns NULL on error.
 */

hash_elem_t* icl_hash_insert(hashtable_t *ht, void* key, void *data) {
    hash_elem_t *curr;
    unsigned int hash_val;
    
    if(!ht || !key)
        return NULL;

    hash_val = (* ht->hash_function)(key) % ht->nbuckets;
    for (curr=ht->buckets[hash_val]; curr != NULL; curr=curr->next)
        if ( ht->hash_key_compare(curr->key, key)) {
            return(NULL); /* key already exists */
        }
    
    /* if key was not found */
    curr = (hash_elem_t*) malloc(sizeof(hash_elem_t));
    if(!curr) {
        return NULL;
    }
    curr->key = key;
    curr->data = data;
    curr->next = ht->buckets[hash_val]; /* add at start */
    
    ht->buckets[hash_val] = curr;
    return curr;
}

/**
 * Free one hash table entry located by key (key and data are freed using functions).
 *
 * @param ht -- the hash table to be freed
 * @param key -- the key of the new item
 * @param free_key -- pointer to function that frees the key
 * @param free_data -- pointer to function that frees the data
 *
 * @returns 0 on success, -1 on failure.
 */
int icl_hash_delete(hashtable_t *ht, void* key, void (*free_key)(void*), void (*free_data)(void*)) {
    hash_elem_t *curr, *prev;
    unsigned int hash_val;
    
    if(!ht || !key)
        return -1;
    
    prev = NULL;
    hash_val = (* ht->hash_function)(key) % ht->nbuckets;
    for (curr=ht->buckets[hash_val]; curr != NULL; )  {
        if ( ht->hash_key_compare(curr->key, key)) {
            if (prev == NULL) {
                ht->buckets[hash_val] = curr->next;
            } else {
                prev->next = curr->next;
            }
            if (*free_key && curr->key) (*free_key)(curr->key);
            if (*free_data && curr->data) (*free_data)(curr->data);
            free(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    return -1;
}

/**
 * Free hash table structures (key and data are freed using functions).
 *
 * @param ht -- the hash table to be freed
 * @param free_key -- pointer to function that frees the key
 * @param free_data -- pointer to function that frees the data
 *
 * @returns 0 on success, -1 on failure.
 */
int icl_hash_destroy(hashtable_t *ht, void (*free_key)(void*), void (*free_data)(void*)) {
    hash_elem_t *bucket, *curr, *next;
    int i;
    
    if(!ht)
        return -1;
    
    for (i=0; i<ht->nbuckets; i++) {
        bucket = ht->buckets[i];
        for (curr=bucket; curr!=NULL; ) {
            next=curr->next;
            if (*free_key && curr->key) (*free_key)(curr->key);
            if (*free_data && curr->data) (*free_data)(curr->data);
            utente_t* user = curr -> data;
            
            history_msg_t* tmp;
            while((tmp = (history_msg_t*)dequeue_2(&user->history)) != NULL) {
                if(tmp) {
                    if(tmp->msg.data.buf)
                        free(tmp->msg.data.buf);
                    free(tmp);
                }
            }
            
            delete_queue(&user -> history);
            free(curr -> data);
            free(curr);
            curr = next;
        }
    }
    if(ht->buckets) free(ht->buckets);
    if(ht) {
        free(ht);
        ht = NULL;
    }
    return 0;
}


void post_txt_all(hashtable_t *ht, int nbuckets, char* sender, message_t msg) {
    hash_elem_t* corr;
    utente_t* user_receiver;
    
    for(int i=0; i<nbuckets; i++) {
        unsigned int hash_val = i % ht->nbuckets;
        pthread_mutex_lock(&ht->locks[hash_val % 8]);
        corr = ht -> buckets[i];
        while(corr != NULL) {
            user_receiver = corr -> data;
            if(strcmp(user_receiver -> nickname, sender) != 0) {
                history_msg_t* message = malloc(sizeof(history_msg_t));
                setHeader(&message -> msg.hdr, TXT_MESSAGE, msg.hdr.sender);
                int size = (int) msg.data.hdr.len;
                char* buf = malloc(size * sizeof(char));
                memcpy(buf, msg.data.buf, size);
                
                setData(&message -> msg.data, corr -> key, buf, size);
                if(user_receiver -> isOnline) {
                    if(send_message(user_receiver -> fd, message -> msg) <= 0)
                        update_statistics(0, 0, 0, 0, 0, 0, 1);
                    else {
                        message -> received = 1;
                        update_statistics(0, 0, 1, 0, 0, 0, 0);
                    }
                }
                else {
                    message -> received = 0;
                    update_statistics(0, 0, 0, 1, 0, 0, 0);
                }
                // Inserisco il messaggio nella history dell'utente
                enqueue(&user_receiver -> history, message);
                
                if(user_receiver -> history_dim < configurations.MaxHistMsgs)
                    user_receiver -> history_dim ++;
                //Se la history è piena, elimino l'elemento più vecchio
                else {
                    history_msg_t* tmp = dequeue_2(&user_receiver -> history);
                    if(tmp) {
                        if(tmp -> msg.data.buf)
                            free(tmp -> msg.data.buf);
                        free(tmp);
                    }
                }
            }
        corr = corr -> next;
        }
        unlock_hash(ht, hash_val);
    }
}


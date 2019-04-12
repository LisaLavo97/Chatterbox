/*
* @file user_manager.c
* @author Lisa Lavorati
* Si dichiara che il contenuto di questo file è in ogni sua parte opera
* originale dell'autore */

#include "user_manager.h"
#include "ops.h"
#include "config.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>


utente_t* create_user(char* nickname, int fd, hashtable_t* utenti_registrati) {
    if(strlen(nickname) > MAX_NAME_LENGTH) {
        errno = EINVAL;
        return NULL;
    }
    utente_t* user = (utente_t*) malloc(sizeof(utente_t));
    memset(user,0,sizeof(utente_t));
    strcpy(user -> nickname, nickname);
    user -> isOnline = 0;
    user -> fd = fd;
    create_queue(&user -> history);
    user -> history_dim = 0;
    return user;
}


user_manager_t* create_user_manager(void) {
    user_manager_t* user_manager = (user_manager_t*) malloc(sizeof(user_manager_t));
    user_manager -> utenti_registrati = icl_create_hashtable(255);
    create_queue(&user_manager -> utenti_connessi);
    
    return user_manager;
}


utente_t* get_utente(hashtable_t* utenti_registrati, char* nickname) {
    return icl_hash_find(utenti_registrati, nickname);
}


void set_online(hashtable_t* utenti_registrati, utente_t* user, int fd) {
    user -> isOnline = 1;
    user -> fd = fd;
}


void set_offline(hashtable_t* utenti_registrati, utente_t* user, int fd) {
    user -> isOnline = 0;
    user -> fd = -1;
}


int register_user(hashtable_t* utenti_registrati, char* nickname, int fd) {
    if(utenti_registrati == NULL || strlen(nickname) <= 0 || fd < 0) {
        errno = EINVAL;
        return OP_FAIL;
    }
    utente_t* user = get_utente(utenti_registrati, nickname);
    
    if(user)
        return OP_NICK_ALREADY;
    
    // Utente non è registrato
    else {
        utente_t* new_user = create_user(nickname, fd, utenti_registrati);
        hash_elem_t* new_elem = icl_hash_insert(utenti_registrati, new_user->nickname, new_user);
        if(new_elem != NULL) {
            printf("Utente inserito tra gli utenti registrati: %s\n", nickname);
            return OP_OK;
        }
    }
    return OP_FAIL;
}


int deregister_user(user_manager_t* user_manager, char* nickname) {
    if(user_manager -> utenti_registrati == NULL || strlen(nickname) <= 0 ) {
        errno = EINVAL;
        return OP_FAIL;
    }
    
    utente_t* user = get_utente(user_manager -> utenti_registrati, nickname);
    int res;
    // Utente precedentemente registrato
    if(user) {
        if(user -> isOnline) {
           res = disconnect_user(user_manager, nickname);
            if(res == OP_OK) {
                res = icl_hash_delete(user_manager -> utenti_registrati, nickname, NULL, NULL);
                if(res == 0)
                    return OP_OK;
            }
            return OP_FAIL;
        }
        else {
            res = icl_hash_delete(user_manager -> utenti_registrati, nickname, NULL, NULL);
            if(res == 0)
                return OP_OK;
            else
                return OP_FAIL;
        }
    }
    // Utente non è registrato
    else
        return OP_NICK_UNKNOWN;
}


int connect_user(user_manager_t* user_manager, char* nickname, int fd) {
    utente_t* user = get_utente(user_manager -> utenti_registrati, nickname);
    if(user) {
        // L'utente è registrato e offline
        if(user -> isOnline == 0) {
            set_online(user_manager -> utenti_registrati, user, fd);
            enqueue(&user_manager -> utenti_connessi, user);
            return OP_OK;
        }
        // L'utente è già connesso
        else
            return OP_NICK_ALREADY;
    }
    // L'utente non è registrato
    else
        return OP_NICK_UNKNOWN;
}


int disconnect_user(user_manager_t* user_manager, char* nickname) {
    utente_t* user = get_utente(user_manager -> utenti_registrati, nickname);
    // L'utente è registrato
    if(user) {
        // L'utente è connesso
        if(user -> isOnline) {
            set_user_offline(&user_manager -> utenti_connessi, user->fd);
            return OP_OK;
        }
        //L'utente non è connesso
        else
            return OP_FAIL;
    }
    // L'utente non è registrato
    else
        return OP_NICK_UNKNOWN;
}


char* get_utenti_connessi(user_manager_t* user_manager, char* nickname, int *dim) {
    if(get_utente(user_manager -> utenti_registrati, nickname)) {
        lock_queue(&user_manager -> utenti_connessi);
        node_t* corr = user_manager -> utenti_connessi.head;
        char* buf = (char*) malloc((user_manager -> utenti_connessi.dim) * (MAX_NAME_LENGTH + 1) * sizeof(char));
        char* ret = buf;
        if(!buf) {
            errno = ENOMEM;
            unlock_queue(&user_manager -> utenti_connessi);
            return NULL;
        }
        int len = 0;
        printf("Lista utenti connessi: \n");
        for(corr = user_manager -> utenti_connessi.head; corr != NULL; corr = corr -> next) {
            char* nickname = corr -> info;
            printf("%s\n", nickname);
            strncpy(buf, nickname, (MAX_NAME_LENGTH + 1));
            len += MAX_NAME_LENGTH + 1;
            buf += MAX_NAME_LENGTH + 1;
        }
        *dim = len;
        unlock_queue(&user_manager -> utenti_connessi);
        return ret;
    }
    else
        return NULL;
}

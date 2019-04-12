/*
 * @file user.h
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#ifndef USER_H_
#define USER_H_

#include <pthread.h>
#include "config.h"
#include "message.h"
#include "queue.h"


/**
 * @struct history_msg
 * @brief struttura contenente un messaggio della history e un bit che identifica se è stato o meno letto dal client
 * @param msg messaggio ricevuto
 * @param received bit che indica se il messaggio è stato letto dal client
 */
 typedef struct history_msg {
    message_t msg;
    int received; // =1 se il messaggio è stato ricevuto, = 0 se è un messaggio pendente
} history_msg_t;


/**
 @struct utente
 @brief contiene le informazioni relative ad un singolo utente di Chatty
 @param nickname identificatore alfanumerico univoco associato all'utente
 @param fd file descriptor associato all'utente
 @param isOnline flag che indica se l'utente è online o meno
 @param history coda dei messaggi ricevuti
 @param history_dim numero di elementi nella history
*/
typedef struct utente {
    char nickname[MAX_NAME_LENGTH + 1];
    int fd;
    int isOnline;
    queue_t history;
    int history_dim;
} utente_t;


#endif /* USER_H_ */

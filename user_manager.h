/*
 @file user_manager.h
 @author Lisa Lavorati
 Si dichiara che il contenuto di questo file è in ogni sua parte opera
 originale dell'autore */

#ifndef user_manager_h
#define user_manager_h

#include "queue.h"
#include "hashtable.h"
#include "user.h"

/**
 @struct user_manager
 @brief struttura contenente le strutture dati relative agli utenti di chatty
 @param utenti_registrati hashtable contenente tutti gli utenti registrati a chatty
 @param utenti_connessi coda contenente tutti gli utenti al momento connessi
 */
typedef struct user_manager {
    hashtable_t* utenti_registrati;
    queue_t utenti_connessi;
    
} user_manager_t;


/*
 * @function create_user
 * @brief Crea una struttura di tipo utente_t
 * @param nickname sequenza alfanumerica che identifica univocamente un utente
 * @param utenti_registrati struttura dati contenente tutti gli utenti registrati
 * @return l'oggetto di tipo utente creato
 */
utente_t* create_user(char* nickname, int fd, hashtable_t* utenti_registrati);


/*
 * @function create_user_manager
 * @brief Crea una struttura di tipo user_manager_t
 * @return l'oggetto di tipo user_manager_t
 */
user_manager_t* create_user_manager(void);


/*
 * @function destroy_user_manager
 * @brief Distrugge la struttura di tipo user_manager_t puntata dall'argomento
 * @param user_manager oggetto da distuggere
 *
 */
void destroy_user_manager(user_manager_t** user_manager);


/*
 * @function register_user
 * @brief Inserisce un utente nella struttura contenente tutti gli utenti registrati
 * @param utenti_registrati struttura dati contenente tutti gli utenti registrati
 * @param nickname sequenza alfanumerica che identifica univocamente un utente
 * @param fd file descriptor relativo del client corrispondente
 * @return
 */
int register_user(hashtable_t* utenti_registrati, char* nickname, int fd);


/*
 * @function deregister_user
 * @brief Elimina un utente dallalla struttura contenente tutti gli utenti registrati
 * @param user_manager gestore degli utenti
 * @param nickname identificatore univoco dell'utente da deregistrare
 * @return l'intero corrispondente al tipo di operazione da inviare al client
 */
int deregister_user(user_manager_t* user_manager, char* nickname);


/*
 * @function connect_user
 * @brief Inserisce un utente nella lista degli utenti online
 * @param user_manager gestore degli utenti
 * @param nickname identificatore univoco dell'utente da connettere
 * @param fd file descriptor relativo del client corrispondente
 * @return l'intero corrispondente al tipo di operazione da inviare al client
 */
int connect_user(user_manager_t* user_manager, char* nickname, int fd);


/*
 * @function disconnect_user
 * @brief Elimina un utente nella lista degli utenti online
 * @param user_manager gestore degli utenti
 * @param nickname identificatore univoco dell'utente da connettere
 * @return l'intero corrispondente al tipo di operazione da inviare al client
 */
int disconnect_user(user_manager_t* user_manager, char* nickname);


/*
 * @function get_utenti_connessi
 * @brief Funzione che restituisce un buffer contenente l'elenco degli utenti connessi al momento
 * @param user_manager gestore degli utenti
 * @param dim puntatore alla dimensione della lista
 * @return un buffer contenente l'elenco degli utenti connessi al momento
 */
char* get_utenti_connessi(user_manager_t* user_manager, char* nickname, int *dim);


/*
 * @function get_utente
 * @brief Funzione che effettua la ricerca di un utente nell'hashtable degli utenti registrati
 * @param utenti_registrati struttura dati contenente tutti gli utenti registrati
 * @param nickname identificatore univoco dell'utente
 * @return la struttura utente associata a tale nickname, o NULL se l'utente non è registrato
 */
utente_t* get_utente(hashtable_t* utenti_registrati, char* nickname);


/*
 * @function set_online
 * @brief Funzione che setta online un utente
 * @param utenti_registrati struttura dati contenente tutti gli utenti registrati
 * @param user utente da settare online
 * @param fd file descriptor relativo del client corrispondente
 */
void set_online(hashtable_t* utenti_registrati, utente_t* user, int fd);


/*
 * @function set_offline
 * @brief Funzione che setta offline un utente
 * @param utenti_registrati struttura dati contenente tutti gli utenti registrati
 * @param user utente da settare offline
 * @param fd file descriptor relativo del client corrispondente
 */
void set_offline(hashtable_t* utenti_registrati, utente_t* user, int fd);


#endif /* user_manager_h */


/*
* @file request_handler.h
* @author Lisa Lavorati
* Si dichiara che il contenuto di questo file è in ogni sua parte opera
* originale dell'autore */

#ifndef REQUEST_HANDLER_H_
#define REQUEST_HANDLER_H_

#include "message.h"
#include "user_manager.h"
#include "configuration.h"


/*
 * @function hand_request
 * @brief Funzione per gestire le richieste dei client
 * @param client_fd file descriptor associato al client che ha fatto la richiesta
 * @param msg messaggio di richiesta inviato dal client
 * @param user_manager gestore degli utenti
 * @return 1 in caso di successo, 0 in caso di fallimento o nel caso il client chiuda
    la connessione durante l'esecuzione della richiesta
 */
int hand_request (int client_fd, message_t msg, user_manager_t* user_manager);


/*
 * @function send_header
 * @brief Funzione che chiama la funzione sendHeader, ma prima controllando se l'utente
    a cui si sta scrivendo è online
 * @rparam fd file descriptor associato al client
 * @hdr message header da inviare al client
 * @return 1 in caso di successo, o un numero <= 0 altrimenti
 */
int send_header(int fd, message_hdr_t hdr);


/*
 * @function send_data
 * @brief Funzione che chiama la funzione sendData, ma prima controllando se l'utente
 a cui si sta scrivendo è online
 * @rparam fd file descriptor associato al client
 * @hdr message data da inviare al client
 * @return 1 in caso di successo, o un numero <= 0 altrimenti
 */
int send_data(int fd, message_data_t data);


/*
 * @function send_message
 * @brief Funzione che chiama la funzione sendHRequest, ma prima controllando se l'utente
 a cui si sta scrivendo è online
 * @rparam fd file descriptor associato al client
 * @hdr message da inviare al client
 * @return 1 in caso di successo, o un numero <= 0 altrimenti
 */
int send_message(int fd, message_t msg);


#endif /* REQUEST_HANDLER_H_ */





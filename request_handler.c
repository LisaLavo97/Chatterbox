/*
 * @file request_handler.c
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#include "request_handler.h"
#include "user.h"
#include "connections.h"
#include "hashtable.h"
#include "stats.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>


void clean_all(void** elem_1, void** elem_2) {
    if(elem_1) {
        if(*elem_1) {
            free(*elem_1);
            *elem_1 = NULL;
        }
    }
    if(elem_2) {
        if(*elem_2) {
            free(*elem_2);
            *elem_2 = NULL;
        }
    }
}


int send_header(int fd, message_hdr_t hdr) {
    int res = 0;
    if(fd > 0)
        res = sendHeader(fd, &hdr);
    return res;
}


int send_data(int fd, message_data_t data) {
    int res = 0;
    if(fd > 0) {
        res = sendData(fd, &data);
    }
    return res;
}


int send_message(int fd, message_t msg) {
    int res = 0;
    if(fd > 0) {
        res = sendRequest(fd, &msg);
    }
    return res;
}


int hand_request (int client_fd, message_t msg, user_manager_t* user_manager) {
    
    int result;
    message_t reply;
    op_t operation = msg.hdr.op;
    char* sender = msg.hdr.sender;
    
    
    switch (operation) {
            
        case REGISTER_OP: {
            int hash_val = lock_hash(user_manager -> utenti_registrati, sender);
            int registration_result = register_user(user_manager -> utenti_registrati, sender, client_fd);
            if(registration_result != OP_OK) {
                printf("Registrazione non riuscita \n");
                setHeader(&reply.hdr, registration_result, sender);
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                return 0;
            }
            printf("Registrazione riuscita \n");
            result = connect_user(user_manager, sender, client_fd);
            if(result != OP_OK) {
                printf("Connect non riuscita \n");
                setHeader(&reply.hdr, result, "SERVER");
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                return 0;
            }
            int len;
            char* buf = get_utenti_connessi(user_manager, sender, &len);
            if(buf) {
                printf("Connect riuscita \n");
                printf("buffer con gli utenti connessi: %s\n", buf);
                setHeader(&reply.hdr, result, "SERVER");
                setData(&reply.data, sender, buf, len);
                int res = send_message(client_fd, reply);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                update_statistics(1, 1, 0, 0, 0, 0, 0);
                clean_all(&buf,NULL);
                if(res <= 0)
                    return res;
                return 1;
            }
            // Errore nell'allocazione del buffer
            else if(!buf && errno == ENOMEM) {
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                clean_all(&buf,NULL);
                return 0;
            }
        }
        
        case CONNECT_OP: {
            int hash_val = lock_hash(user_manager -> utenti_registrati, sender);
            result = connect_user(user_manager, sender, client_fd);
            
            if(result == OP_NICK_UNKNOWN) {
                printf("Utente sconosciuto \n");
                setHeader(&reply.hdr, result, "SERVER");
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                return 0;
            }
            if(result == OP_NICK_ALREADY) {
                printf("L'utente era già connesso \n");
                setHeader(&reply.hdr, result, "SERVER");
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                return 0;
            }
            int len;
            char* buf = get_utenti_connessi(user_manager, sender, &len);
            if(buf) {
                printf("Connect riuscita \n");
                setHeader(&reply.hdr, result, "SERVER");
                setData(&reply.data, sender, buf, len);
                int res = send_message(client_fd, reply);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                update_statistics(0, 1, 0, 0, 0, 0, 0);
                clean_all(&buf,NULL);
                if(res <= 0)
                    return res;
                return 1;
            }
            // Utente non registrato
            else if(!buf && errno != ENOMEM) {
                setHeader(&reply.hdr, OP_NICK_UNKNOWN, "SERVER");
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                clean_all(&buf,NULL);
                return 0;
            }
            // Errore nell'allocazione del buffer
            else if(!buf && errno == ENOMEM) {
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                return 0;
            }
        }
            
        case POSTTXT_OP: {
            // Messaggio troppo lungo
            if (msg.data.hdr.len > configurations.MaxMsgSize) {
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                setHeader(&reply.hdr, OP_MSG_TOOLONG, "SERVER");
                int hash_val = lock_hash(user_manager -> utenti_registrati, sender);
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                return 0;
            }
            
            char* receiver = msg.data.hdr.receiver;
            int hash_val_receiver = lock_hash(user_manager -> utenti_registrati, receiver);
            utente_t* user_receiver = get_utente(user_manager -> utenti_registrati, receiver);
            
            if(user_receiver) {
                history_msg_t* message = malloc(sizeof(history_msg_t));
                setHeader(&message -> msg.hdr, TXT_MESSAGE, msg.hdr.sender);
                
                int size = (int) msg.data.hdr.len;
                char* buf = malloc(size * sizeof(char));
                memcpy(buf, msg.data.buf, size);
                setData(&message -> msg.data, msg.data.hdr.receiver, buf, size);
                
                // Se il ricevente è online allora gli inoltro il messaggio direttamente
                if(user_receiver -> isOnline) {
                    int res = send_message(user_receiver -> fd, message -> msg);
                    printf("Messaggio: %s\n", message -> msg.data.buf);
                    // Il client si è disconnesso, non ha ricevuto il messaggio
                    if(res == 0)
                        message -> received = 0;
                    if(res < 0) {
                        printf("Non riesco a inviare il msg all'utente online \n");
                        unlock_hash(user_manager -> utenti_registrati, hash_val_receiver);
                        return 0;
                    }
                    else {
                        printf("Messaggio inviato all'utente online \n");
                        message -> received = 1;
                    }
                }
                else
                    message -> received = 0;
                    
                // Metto nella history il MESSAGGIO ricevuto
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
                unlock_hash(user_manager -> utenti_registrati, hash_val_receiver);
                if(message -> received)
                    update_statistics(0, 0, 1, 0, 0, 0, 0);
                else update_statistics(0, 0, 0, 1, 0, 0, 0);
                
                // Mando l'ack all'utente mittente
                setHeader(&reply.hdr, OP_OK, "SERVER");
                int hash_val_sender = lock_hash(user_manager -> utenti_registrati, sender);
                int send = send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val_sender);
                return send;
            }
            else {
                unlock_hash(user_manager -> utenti_registrati, hash_val_receiver);
                setHeader(&reply.hdr, OP_NICK_UNKNOWN, sender);
                int hash_val_sender = lock_hash(user_manager -> utenti_registrati, sender);
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val_sender);
                return 0;
            }
        }
            
        case POSTTXTALL_OP: {
            int hash_val;
            // Messaggio troppo lungo
            if (msg.data.hdr.len > configurations.MaxMsgSize) {
                setHeader(&reply.hdr, OP_MSG_TOOLONG, "SERVER");
                hash_val = lock_hash(user_manager -> utenti_registrati, sender);
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                return 0;
            }
            
            hash_val = lock_hash(user_manager -> utenti_registrati, sender);
            utente_t* user_sender = get_utente(user_manager -> utenti_registrati, sender);
            unlock_hash(user_manager -> utenti_registrati, hash_val);
            if(user_sender) {
                // Invio il messaggio a tutti gli utenti registrati
                post_txt_all(user_manager -> utenti_registrati, user_manager -> utenti_registrati -> nbuckets, sender, msg);
                // Invio il responso al mittente
                setHeader(&reply.hdr, OP_OK, sender);
                hash_val = lock_hash(user_manager -> utenti_registrati, sender);
                int res = send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                return res;
            }
            else {
                setHeader(&reply.hdr, OP_NICK_UNKNOWN, sender);
                hash_val = lock_hash(user_manager -> utenti_registrati, sender);
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                return 0;
            }
        }
            
        case POSTFILE_OP: {
            char* receiver = msg.data.hdr.receiver;
            int size = (int) strlen(msg.data.buf) + 1;
            char* file_name = malloc(size * sizeof(char));
            memset(file_name, 0, size);
            strncpy(file_name, msg.data.buf, size);
            int err, hash_val_sender, hash_val_receiver;
            message_data_t file_data;
            if((err = readData(client_fd, &file_data)) <= 0) {
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                clean_all(&file_name, NULL);
                return err;
            }
            
            if (file_data.hdr.len > configurations.MaxFileSize * 1024) {
                // Invio errore troppo lungo
                setHeader(&reply.hdr, OP_MSG_TOOLONG, "SERVER");
                hash_val_sender = lock_hash(user_manager -> utenti_registrati, sender);
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val_sender);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                clean_all(&file_name, &file_data.buf);
                return 0;
            }
            
            char* name = malloc(size * sizeof(char));
            char* copy = name;
            memset(name, 0, size);
            strcpy(name, file_name);
            char* tmp = strtok(name, "/");
            while(name) {
                tmp = name;
                name = strtok(NULL, "/");
            }
            strcpy(file_name, tmp);
            free(copy);
            char dir[UNIX_PATH_MAX];
            strcpy(dir, configurations.DirName);
            strcat(dir, "/");
            strcat(dir, file_name);
            FILE* file = fopen(dir, "w");
            if(file == NULL){
                perror("open");
                fprintf(stderr, "Errore nell'apertura del file  %s\n", file_name);
                setHeader(&reply.hdr, OP_NO_SUCH_FILE, "SERVER");
                hash_val_sender = lock_hash(user_manager -> utenti_registrati, sender);
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val_sender);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                clean_all(&file_data.buf, NULL);
                return 0;
            }
            
            if (fwrite(file_data.buf, sizeof(char), file_data.hdr.len, file) != file_data.hdr.len) {
                perror("write");
                fprintf(stderr, "Errore nella scrittura del file  %s\n", file_name);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                fclose(file);
                clean_all(&file_data.buf, NULL);
                return 0;
            }
            
            hash_val_receiver = lock_hash(user_manager -> utenti_registrati, receiver);
            utente_t* user_receiver = get_utente(user_manager -> utenti_registrati, receiver);
            history_msg_t* message = malloc(sizeof(history_msg_t));
            setHeader(&message -> msg.hdr, FILE_MESSAGE, msg.hdr.sender);
            setData(&message -> msg.data, receiver, file_name, size);
            printf("File sender:  %s\n", msg.hdr.sender);
            printf("File receiver:  %s\n", receiver);
            printf("File name:  %s\n", file_name);
            
            // Metto nella history il FILE_MESSAGE ricevuto
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
            // Se è online, invio la notifica di file ricevuto al client ricevente
            if(user_receiver -> isOnline) {
                int res = send_message(user_receiver -> fd, message -> msg);
                unlock_hash(user_manager -> utenti_registrati, hash_val_receiver);
                if(res <= 0)
                    clean_all(&file_data.buf, NULL);
                else
                    update_statistics(0, 0, 0, 0, 1, 0, 0);
            }
            else {
                unlock_hash(user_manager -> utenti_registrati, hash_val_receiver);
                update_statistics(0, 0, 0, 0, 0, 1, 0);
            }
            
            //Invio responso al client mittente
            setHeader(&reply.hdr, OP_OK, "SERVER");
            hash_val_sender = lock_hash(user_manager -> utenti_registrati, sender);
            int send = send_header(client_fd, reply.hdr);
            unlock_hash(user_manager -> utenti_registrati, hash_val_sender);
            clean_all(&file_data.buf, NULL);
            fclose(file);
            if(send <= 0)
                return send;
            return 1;
        }
            
        case GETFILE_OP: {
            int size = (int) strlen(msg.data.buf) + 1;
            char* file_name = malloc(size * sizeof(char));
            memset(file_name, 0, size);
            strcpy(file_name, msg.data.buf);
            
            char* name = malloc(size * sizeof(char));
            char* copy = name;
            memset(name, 0, size);
            strcpy(name, file_name);
            char* tmp = strtok(name, "/");
            while(name) {
                tmp = name;
                name = strtok(NULL, "/");
            }
            strcpy(file_name, tmp);
            free(copy);
            printf("Filename:  %s\n", file_name);
            char dir[UNIX_PATH_MAX];
            strcpy(dir, configurations.DirName);
            strcat(dir, "/");
            strcat(dir, file_name);
            printf("Filepath:  %s\n", dir);
            
            struct stat st;
            if(stat(file_name, &st) < 0) {
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                clean_all(&file_name, NULL);
                return 0;
            }
            
            int fd = open(dir, O_RDONLY);
            if(fd < 0){
                // Il file non esiste
                perror("open");
                fprintf(stderr, "Errore nell'apertura del file  %s\n", file_name);
                close(fd);
                setHeader(&reply.hdr, OP_NO_SUCH_FILE, "SERVER");
                int hash_val = lock_hash(user_manager -> utenti_registrati, sender);
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                clean_all(&file_name, NULL);
                return 0;
            }
           
            size_t file_size = st.st_size;
            char* mappedfile = NULL;
            // Mappo il file da spedire in memoria
            mappedfile = mmap(NULL, file_size, PROT_READ,MAP_PRIVATE, fd, 0);
            if (mappedfile == MAP_FAILED) {
                perror("mmap");
                fprintf(stderr, "Errore mappando il file %s in memoria\n", file_name);
                close(fd);
                clean_all(&file_name, NULL);
                return 0;
            }
            close(fd);

            setHeader(&reply.hdr, OP_OK, "SERVER");
            int hash_val = lock_hash(user_manager -> utenti_registrati, sender);
            int err = send_header(client_fd, reply.hdr);
            if(err <= 0) {
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                clean_all(&file_name, NULL);
                return err;
            }
            // Invio i dati
            setData(&reply.data, "", mappedfile, (unsigned int) file_size);
            err = send_data(client_fd, reply.data);
            unlock_hash(user_manager -> utenti_registrati, hash_val);
            clean_all(&file_name, NULL);
            if(err <= 0)
                return err;
            return 1;
        }
            
        case GETPREVMSGS_OP: {
            int hash_val = lock_hash(user_manager -> utenti_registrati, sender);
            utente_t* user = get_utente(user_manager -> utenti_registrati, sender);
            char* buf = (char*) &user -> history_dim;
            setHeader(&reply.hdr, OP_OK, "SERVER");
            int res = send_header(client_fd, reply.hdr);
            if(res <= 0) {
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                return res;
            }
            setData(&reply.data, sender, buf, sizeof(user -> history_dim));
            res = send_data(client_fd, reply.data);
            if(res <= 0) {
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                return res;
            }
            int no_more_pendenti = 0;
            node_t* corr = user -> history.head;
            while(corr != NULL) {
                history_msg_t* tmp = corr-> info;
                printf("Messaggio: %s\n", tmp -> msg.data.buf);
                res = send_message(client_fd, tmp -> msg);
                if(res <= 0) {
                    unlock_hash(user_manager -> utenti_registrati, hash_val);
                    update_statistics(0, 0, no_more_pendenti, -no_more_pendenti, 0, 0, 0);
                    return res;
                }
                if(!tmp -> received) {
                    no_more_pendenti++;
                    tmp -> received = 1;
                }
                corr = corr -> next;
            }
            if(no_more_pendenti != 0)
                update_statistics(0, 0, no_more_pendenti, -no_more_pendenti, 0, 0, 0);
            unlock_hash(user_manager -> utenti_registrati, hash_val);
            return 1;
        }
            
        case USRLIST_OP: {
            int len;
            int hash_val = lock_hash(user_manager -> utenti_registrati, sender);
            char* buf = get_utenti_connessi(user_manager, sender, &len);
            if(buf) {
                setHeader(&reply.hdr, OP_OK, "SERVER");
                setData(&reply.data, sender, buf, len);
                int res = send_message(client_fd, reply);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                clean_all(&buf, NULL);
                return res;
            }
            // Errore nell'allocazione del buffer
            else if(!buf && errno == ENOMEM) {
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                return 0;
            }
            // Utente non è registrato
            else if(!buf && errno != ENOMEM) {
                setHeader(&reply.hdr, OP_NICK_UNKNOWN, "SERVER");
                send_header(client_fd, reply.hdr);
                unlock_hash(user_manager -> utenti_registrati, hash_val);
                update_statistics(0, 0, 0, 0, 0, 0, 1);
                clean_all(&buf, NULL);
                return 0;
            }
        }
            
        case UNREGISTER_OP: {
            int hash_val = lock_hash(user_manager -> utenti_registrati, sender);
            result = deregister_user(user_manager, sender);
            setHeader(&reply.hdr, result, "SERVER");
            send_header(client_fd, reply.hdr);
            unlock_hash(user_manager -> utenti_registrati, hash_val);
            if(result == OP_OK)
                update_statistics(-1, 0, 0, 0, 0, 0, 0);
            return 0;
        }
            
        case DISCONNECT_OP: {
            int hash_val = lock_hash(user_manager -> utenti_registrati, sender);
            result = disconnect_user(user_manager, sender);
            setHeader(&reply.hdr, result, "SERVER");
            setData(&reply.data, sender, NULL, 0);
            send_message(client_fd, reply);
            unlock_hash(user_manager -> utenti_registrati, hash_val);
            return 0;
        }
            
        default:
            break;
    }
    
    return 0;
}





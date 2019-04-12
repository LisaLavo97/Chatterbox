/*
 * @file listener.c
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#include "listener.h"
#include "queue.h"
#include "message.h"
#include "config.h"
#include "stats.h"
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/select.h>

void start_listener(threadpool_t* pool, user_manager_t* user_manager) {
    
    if(unlink(configurations.UnixPath) < 0 )
        errno = 0;
    
    
    extern struct statistics chattyStats;
    extern pthread_mutex_t stats_mutex;
    extern int client_connessi;
    extern pthread_mutex_t mutex_connessi;
    extern volatile sig_atomic_t sigalarm_flag;

    
    int fd_socket, fd_client, fd_num = 0, fd;
    fd_set set, rdset;
    
    struct sockaddr_un sa;
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, configurations.UnixPath, UNIX_PATH_MAX);
    
    fd_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    
    if(bind(fd_socket,(struct sockaddr*) &sa, sizeof(sa)) == -1) {
        perror("bind");
        fprintf(stderr, "Errore nella bind \n");
        return;
    }
  
    if(listen(fd_socket, SOMAXCONN) == -1) {
        perror("select");
        fprintf(stderr, "Errore nel listen \n");
        return;
    }
    
    /* mantengo il massimo indice di descrittore attivo in fd_num */
    if (fd_socket > fd_num)
         fd_num = fd_socket;
    FD_ZERO(&set);
    FD_SET(fd_socket, &set);
    
    while (!sigalarm_flag) {
        int tmp_fd;
        while((tmp_fd = (int)dequeue_2(&pool -> fd_queue_2)) != NULL) {
            FD_SET(tmp_fd, &set);
            if (tmp_fd > fd_num)
                fd_num = tmp_fd;
        }
        
        rdset = set;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10 * 1000;
    
        if (select(fd_num + 1, &rdset, NULL, NULL, &tv) == - 1) {
            perror("select");
            fflush(stdout);
            continue;
        }
        /* select OK */
        else {
            for (fd = 0; fd <= fd_num; fd++) {
                if(!FD_ISSET(fd, &rdset))
                   continue;
                   
                else {
                    /* sock connect pronto */
                    if (fd == fd_socket) {
                        // Controllo se ho raggiunto il numero massimo di connessioni
                        int accept_conn = 0;
                        pthread_mutex_lock(&mutex_connessi);
                        int connessi = client_connessi;
                        pthread_mutex_unlock(&mutex_connessi);
                        if(connessi >= configurations.MaxConnections) {
                            printf("MaxConnections raggiunto \n");
                            fflush(stdout);
                            continue;
                        }
                        else accept_conn = 1;
                        if(accept_conn) {
                            fd_client = accept(fd_socket, NULL, 0);
                            pthread_mutex_lock(&mutex_connessi);
                            client_connessi ++;
                            pthread_mutex_unlock(&mutex_connessi);
                            printf("Nuova connessione accettata, FD: %d\n", fd_client);
                            fflush(stdout);
                            FD_SET(fd_client, &set);
                            if (fd_client > fd_num)
                                fd_num = fd_client;
                        }
                    }
                    
                    /* sock I/0 pronto */
                    else {
                        printf("Nuova richiesta \n");
                        //Push del fd nella coda di fd condivisa tra threadpool e listener
                        if(enqueue(&pool -> fd_queue_1, (int*) fd)) {
                            FD_CLR(fd, &set);
                            while(!FD_ISSET(fd_num, &set))
                                fd_num --;
                        }
                        else {
                            perror("enqueue");
                            fprintf(stderr, "Errore nell'inserimento del fd nella coda condivisia tra threadpool e listener \n");
                        }
                    }
                }
            }
        }
    }

    if(sigalarm_flag) {
        printf("Avvio chiusura del server \n");
        //Distruggo il threadpool
        destroy_pool(pool, configurations.ThreadsInPool);

        //Distruggo la struttura contenente gli utenti registrati
        icl_hash_destroy(user_manager -> utenti_registrati, NULL, NULL);

        //Distruggo la struttura contenente gli utenti connessi
        delete_queue(&user_manager -> utenti_connessi);
        
        //Distruggo l'usermanager
        free(user_manager);
        user_manager = NULL;
    }
}

/*
 *
 * @file worker.c
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#include "worker.h"
#include "queue.h"
#include "user.h"
#include "message.h"
#include "stats.h"
#include "connections.h"
#include "request_handler.h"
#include <unistd.h>
#include <signal.h>
#include <errno.h>

void* worker (threadpool_t* pool) {
    
    if(pool == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    message_t msg;
    memset(&msg, 0, sizeof(message_t));
    extern int client_connessi;
    extern pthread_mutex_t mutex_connessi;
    extern volatile sig_atomic_t sigalarm_flag;

    while(1) {
        int fd = (int) dequeue_1(&pool -> fd_queue_1);
        if(fd == -1)
            break;
        
        int err = readMsg((int) fd, &msg);
        
        if(err > 0) {
            err = hand_request((int) fd, msg, pool -> user_manager);
            
            if(msg.data.buf) {
                free(msg.data.buf);
                msg.data.buf = NULL;
            }
        }
        
        if(err > 0)
            enqueue(&pool -> fd_queue_2, (int*) fd);
        else {
            set_user_offline(&(pool -> user_manager -> utenti_connessi), fd);
            pthread_mutex_lock(&mutex_connessi);
            client_connessi --;
            pthread_mutex_unlock(&mutex_connessi);
            close(fd);
        }
    }
    return NULL;
}


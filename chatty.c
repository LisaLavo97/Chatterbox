/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 * @file chatty.c
 * @brief File principale del server chatterbox
 * @author Lisa Lavorati
 Si dichiara che il contenuto di questo file è in ogni sua parte opera
 originale dell'autore */

#define _POSIX_C_SOURCE 200809L
#include "threadpool.h"
#include "queue.h"
#include "listener.h"
#include "worker.h"
#include "stats.h"
#include "user.h"
#include "user_manager.h"
#include "configuration.h"
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>



/* struttura che memorizza le statistiche del server, struct statistics 
 * e' definita in stats.h */
struct statistics chattyStats = { 0,0,0,0,0,0,0 };

// Mutex per l'aggiornamento delle statistiche
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// Struttura dove vengono memorizzate le configurazioni del server
config_t configurations = {NULL, 0, 0, 0, 0, 0,NULL, NULL};

int client_connessi = 0;

// Mutex per l'aggiornamento del numero di utenti connessi
pthread_mutex_t mutex_connessi = PTHREAD_MUTEX_INITIALIZER;

volatile sig_atomic_t sigalarm_flag = 0;


static void usage(const char *progname) {
    fprintf(stderr, "Il server va lanciato con il seguente comando:\n");
    fprintf(stderr, "  %s -f conffile\n", progname);
}


void print_statistics() {
    FILE *file_stats;
    file_stats = fopen(configurations.StatFileName, "w");
    if (file_stats) {
        pthread_mutex_lock(&stats_mutex);
        print_chatty_stats(file_stats);
        pthread_mutex_unlock(&stats_mutex);
    }
    fclose(file_stats);
}


void stop_server() {
    sigalarm_flag = 1;
}


int signals_handler() {
    // Dichiaro le strutture
    struct sigaction exit_handler;
    struct sigaction stats_handler;
    struct sigaction pipe_handler;
    
    // Azzero il cotenuto delle strutture
    memset(&exit_handler, 0, sizeof(exit_handler));
    memset(&stats_handler, 0, sizeof(stats_handler));
    memset(&pipe_handler, 0, sizeof(pipe_handler));
    
    // Assegno le funzioni handler alle rispettive strutture
    exit_handler.sa_handler = stop_server;
    stats_handler.sa_handler = print_statistics;
    pipe_handler.sa_handler = SIG_IGN;
    
    // Registro i segnali per terminare il server
    if (sigaction(SIGINT, &exit_handler, NULL) == -1) {
        perror("sigaction");
        return -1;
    }
    if (sigaction(SIGQUIT, &exit_handler, NULL) == -1) {
        perror("sigaction");
        return -1;
    }
    if (sigaction(SIGTERM, &exit_handler, NULL) == -1) {
        perror("sigaction");
        return -1;
    }
    
    // ignoro SIGPIPE per evitare di essere terminato da una scrittura su un socket chiuso
    if (sigaction(SIGPIPE, &pipe_handler, NULL) == -1) {
        perror("sigaction");
        return -1;
    }
    
    // Registro SIGUSR1
    if (configurations.StatFileName && strlen(configurations.StatFileName) > 0) {
        if (sigaction(SIGUSR1, &stats_handler, NULL) == -1) {
            perror("sigaction");
            return -1;
        }
    }
    return 0;
}


int main(int argc, char* argv[]) {
    // Controllo uso corretto degli argomenti
    if (argc < 3 || strncmp(argv[1], "-f", 2) != 0) {
        usage(argv[0]);
        return 0;
    }
    
    threadpool_t* pool = NULL;
    
    configurations = parse(configurations, argv[2]);
    
    user_manager_t* user_manager = create_user_manager();
    if(!user_manager)
        fprintf(stderr, "Errore nella creazione dello user manager \n");
    
    // Funzione per la gestione dei segnali
    signals_handler();
    
    pool = threadpool_init(configurations.ThreadsInPool, worker, user_manager);
    
    start_listener(pool, user_manager);
    
    return 0;
}




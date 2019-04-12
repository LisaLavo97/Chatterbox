/*
 * @file listener.h
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#ifndef LISTENER_H_
#define LISTENER_H_

#include "threadpool.h"
#include "user_manager.h"
#include "configuration.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>


void start_listener (threadpool_t* pool, user_manager_t* user_manager);

#endif  /* LISTENER_H_ */

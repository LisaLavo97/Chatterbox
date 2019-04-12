
/*
 * @file worker.h
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#ifndef WORKER_H_
#define WORKER_H_

#include "threadpool.h"
#include "user_manager.h"
#include "configuration.h"


/*
 * @function worker
 * @brief Funzione principale eseguita dai thread worker
 * @param pool threadpool
 */
void* worker (threadpool_t* pool);

#endif /* WORKER_H_ */

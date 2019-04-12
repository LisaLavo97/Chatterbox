/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 */
/**
 * @file config.h
 * @brief File contenente alcune define con valori massimi utilizzabili
 */

#if !defined(CONFIG_H_)
#define CONFIG_H_

#define MAX_NAME_LENGTH 32

#define MAX_DIM_TEXT_MSG 512

#define MAX_DIM_FILE_MSG 1024

#define NUM_THREADS 8

#define MAX_CONN 32

#define MAX_DIM_HISTORY 100

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

#define UNIX_PATH   "/tmp/chatty_socket"
#define STAT_PATH  "/tmp/chatty_stats.txt"
#define DIR_PATH  "/tmp/chatty"

/* aggiungere altre define qui */



// to avoid warnings like "ISO C forbids an empty translation unit"
typedef int make_iso_compilers_happy;

#endif /* CONFIG_H_ */

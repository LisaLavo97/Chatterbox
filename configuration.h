/*
 * @file configuration.h
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#ifndef configuration_h
#define configuration_h

typedef struct config {
    char* UnixPath;
    int MaxConnections;
    int ThreadsInPool;
    int MaxMsgSize;
    int MaxFileSize;
    int MaxHistMsgs;
    char* DirName;
    char* StatFileName;
} config_t;

extern config_t configurations;

config_t parse(config_t configurations, char* config);


#endif /* configuration_h */

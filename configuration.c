/*
 * @file configuration.c
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#include "configuration.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define dim 512


config_t parse(config_t configurations, char* config) {
    FILE* file;
    file = fopen(config, "r");
    if(file == NULL) {
        perror("open");
        fprintf(stderr, "Errore nell'apertura del file  %s\n", config);
    }
    
    char nome[dim];
    char valore[dim];
    char buf[dim];
    
    while(fgets(buf, dim, file) != NULL) {
        int i = 0, j = 0;
       
        if(buf[i] == '#'|| buf[i] == ' '|| buf[i] == '\n' || buf[i] == '\0')
            continue;
        else {
            while(buf[i] != '=') {
                nome[i] = buf[i];
                i++;
            }
            i += 2;
            while(buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\0' && buf[i] != EOF ) {
                valore[j] = buf[i];
                i++;
                j++;
            }
        }
        i = 0;
        while(isalnum(nome[i]))
            i++;
        nome[i] = '\0';
        valore[j] = '\0';
        
        // PARSING
        if(strcmp(nome, "UnixPath") == 0) {
            if(!configurations.UnixPath)
                configurations.UnixPath = malloc((strlen(valore)+1)*sizeof(char));
            strncpy(configurations.UnixPath, valore, strlen(valore)+1);
        }
        
        else if(strcmp(nome, "DirName") == 0) {
            if(!configurations.DirName)
                configurations.DirName = malloc((strlen(valore)+1)*sizeof(char));
            strncpy(configurations.DirName, valore, strlen(valore)+1);
        }
        
        else if(strcmp(nome, "StatFileName") == 0) {
            if(!configurations.StatFileName)
                configurations.StatFileName = malloc((strlen(valore)+1)*sizeof(char));
            strncpy(configurations.StatFileName, valore, strlen(valore)+1);
        }
        
        else if(strcmp(nome, "MaxConnections") == 0) {
            configurations.MaxConnections = atoi(valore);
        }
        
        else if(strcmp(nome, "ThreadsInPool") == 0) {
            configurations.ThreadsInPool = atoi(valore);
        }
        
        else if(strcmp(nome, "MaxMsgSize") == 0) {
            configurations.MaxMsgSize = atoi(valore);
        }
        
        else if(strcmp(nome, "MaxFileSize") == 0) {
            configurations.MaxFileSize = atoi(valore);
        }
        
        else if(strcmp(nome, "MaxHistMsgs") == 0) {
            configurations.MaxHistMsgs = atoi(valore);
        }
        
    }
    fclose(file);
    return configurations;
}

/*
 * @file connections.c
 * @author Lisa Lavorati
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera
 * originale dell'autore */

#include "message.h"
#include "connections.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>


int openConnection(char* path, unsigned int ntimes, unsigned int secs) {
    if(path == NULL || secs < 0 || ntimes <= 0) {
        errno = EINVAL;
        return -1;
    }
    
    int fd_skt, retry = 0;
    
    // descrittore del socket per la comunicazione con il server
    fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd_skt == -1)
        return -1;
    
    // inizializzo indirizzo
    struct sockaddr_un sa;
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, path, UNIX_PATH_MAX);
    
    while(connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) == -1 && retry <= ntimes) {
        sleep(secs);
        retry ++;
     }
    
    if(retry > ntimes)
        return -1;
    
    return fd_skt;
}


int my_read(long fd, void* buf, unsigned int len) {
    if(fd < 0 || buf == NULL || len <= 0) {
        errno = EINVAL;
        return -1;
    }
    
    int nread, tot_read = 0;
   
    while(len > 0) {
        nread = read(fd, buf + tot_read, len);
        
        if(nread == 0)
            return 0;
        
        if(nread < 0 && errno != EINTR)
            return -1;
        
        if(nread > 0) {
            tot_read += nread;
            len -= nread;
        }
    }
    
    return tot_read;
}


int my_write(long fd, void* buf, unsigned int len) {
    if(fd < 0 || buf == NULL || len <= 0) {
        errno = EINVAL;
        return -1;
    }
    
    int nwrote, tot_wrote = 0;
    
    while(len > 0) {
        nwrote = write(fd, buf + tot_wrote, len);
        if(nwrote == 0)
            return 0;
        
        if(nwrote < 0 && errno != EINTR)
            return -1;
        
        if(nwrote > 0) {
            tot_wrote += nwrote;
            len -= nwrote;
        }
    }
    return tot_wrote;
}


int readHeader(long connfd, message_hdr_t *hdr) {
    if(connfd < 0 || hdr == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    int nread;
    nread = my_read(connfd, hdr, sizeof(message_hdr_t));
    
    return nread;
}


int readData(long fd, message_data_t *data) {
    if(fd < 0 || data == NULL) {
        errno = EINVAL;
        return -1;
    }

    int nread;
    
    // Leggo header del body del messaggio
    nread = my_read(fd, &data -> hdr, sizeof(message_data_hdr_t));
    if(nread <= 0)
        return nread;
    
    data -> buf = NULL;

    if(data -> hdr.len != 0) {
        data -> buf = (char *) malloc(data -> hdr.len * sizeof(char));
        nread = my_read(fd, data -> buf, data -> hdr.len);
        if(nread <= 0) {
            free(data -> buf);
            data -> buf = NULL;
            return nread;
        }
    }
    return 1;
}


int readMsg(long fd, message_t *msg) {
    if(fd < 0 || msg == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    int nread;
    
    nread = readHeader(fd, &msg -> hdr);
    if(nread <= 0)
        return nread;
    
    nread = readData(fd, &msg -> data);
    if(nread <= 0)
        return nread;
    
    return 1;
}


int sendHeader(long fd, message_hdr_t *msg) {
    if(fd < 0 || msg == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    int nwrite;
    nwrite = my_write(fd, msg, sizeof(message_hdr_t));
    
    return nwrite;
}


int sendData(long fd, message_data_t *msg) {
    if(fd < 0 || msg == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    int nwrote;
    nwrote = my_write(fd, &msg -> hdr, sizeof(message_data_hdr_t));
    if(nwrote <= 0)
        return nwrote;
    
    if(msg -> hdr.len != 0 && msg -> buf) {
        nwrote = my_write(fd, msg -> buf, msg -> hdr.len);
        if(nwrote <= 0)
            return nwrote;
    }
    return 1;
}


int sendRequest(long fd, message_t *msg) {
    if(fd < 0 || msg == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    int ris;
    ris = sendHeader(fd, &msg -> hdr);
    if(ris <= 0)
        return ris;
    ris = sendData(fd, &msg -> data);
    
    return ris;
}

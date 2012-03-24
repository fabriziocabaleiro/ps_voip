/* 
 * File:   main.cpp
 * Author: Fabrizio Cabaleiro
 * VoIP, system programming project
 * 
 * Created on March 24, 2012, 4:08 PM
 */

#include <cstdlib>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <pthread.h>

using namespace std;
int *opt = new int[6];

void * monServer(void *arg)
{
    char *buf = new char[30];
    int sfd = *((int*)arg);
    int n;
    listen(sfd,5);
    printf("Monitor Server Running\n");
    for (;;) 
    {
        int session_fd = accept(sfd, 0, 0);
        if(session_fd == -1)                  // session Error
            printf("Error trying to accept connection\n");
        else
        {
            n = recv(session_fd, buf, 30*sizeof(*buf), 0);
            printf("Comentario provicional numero de caracteres recividos: %d",n);
        }
        
    }
}

int main(int argc, char** argv)
{
    pthread_t pt;                            // Thread for Server 
    const int BUF_SIZE = 500;                
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int rsfd;                              // remote socket file descriptor
    int lsfd;                              // local socket file descriptor
    int s, n;
    char buf[BUF_SIZE];
     
    for(int i = 0; i < 6; i++)
        *(opt + i) = 0;

    if (argc != 2)                          // Local port used for own server
    {                                       
        printf("Not enough Parameters\n");  
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family    = AF_UNSPEC;              // Allow IPv4 or IPv6 
    hints.ai_socktype  = SOCK_STREAM;            // TCP 
    hints.ai_flags     = AI_PASSIVE;             // For wildcard IP address   
    hints.ai_protocol  = 0;                      // Any protocol
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;

    
    
    /* Setting up Server */
    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /*  * getaddrinfo() returns a list of address structures.     *
        * Try each address until we successfully bind(2).         *
        * If socket(2) (or bind(2)) fails, we (close the socket   *
        * and) try the next address.                              */

    for (rp = result; rp != NULL; rp = rp->ai_next) 
    {
        lsfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lsfd == -1)
            continue;
        if (bind(lsfd, rp->ai_addr, rp->ai_addrlen) == 0)        // bind ready
            break;                                              
        close(lsfd);
    }
    if (rp == NULL) {                                          /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }
    
    // The server start to run on a separate thread
    pthread_create(&pt, NULL, monServer, &lsfd);   
    
    
    // TODO
    // An easy way to give a server IP, port

    return 0;
}


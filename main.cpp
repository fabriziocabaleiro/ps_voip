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

const int msize = 64;


void * monServer(void *arg)
{
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;

    int nread;
    int fd[2];
    pipe(fd);
    pid_t pid;
    pid = fork();
    if(pid == 0)
    {
        dup2(fd[0],0);
        execlp("play", "play","-", NULL);
        perror("exec");
        _exit(127);
    }
    char *buf = (char*)malloc(msize * sizeof(char));
    int sfd = *((int*)arg);
    printf("\nVoIP Server Running\n");
    for(;;)
    {
        peer_addr_len = sizeof(struct sockaddr_storage);
        nread = recvfrom(sfd, buf, msize, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
        if(nread == -1)
            continue;
        write(fd[1], buf, msize);
    }
}

int main(int argc, char** argv)
{
    char *buff  = (char*)malloc(sizeof(char));
    char *voice = (char*)malloc(msize * sizeof(char));
    char *addr = (char*)malloc(40 * sizeof(char));
    char *port = (char*)malloc(6  * sizeof(char));
    pthread_t pt;                            // Thread for Server         
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int rsfd;                              // remote socket file descriptor
    int lsfd;                              // local socket file descriptor
    int s;
    int i;
    int n = 0;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family    = AF_UNSPEC;              // Allow IPv4 or IPv6 
    hints.ai_socktype  = SOCK_DGRAM;            // UDP 
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
    
    freeaddrinfo(result);
    
    // The server start to run on a separate thread
    pthread_create(&pt, NULL, monServer, &lsfd);   
    
    for(;;)
    {
        printf("IP Address: ");
        scanf("%s",addr);
        printf("Port Server: ");
        scanf("%s",port);
        
        printf("addr: %s port: %s\n",addr, port);
        
        s = getaddrinfo(addr, port, &hints, &result);
        
        if (s != 0) 
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(EXIT_FAILURE);
        }
  
        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
            rsfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
            if (rsfd == -1)
                continue;
            if (connect(rsfd, rp->ai_addr, rp->ai_addrlen) != -1)
                break;                                              // Success 
            close(rsfd);
        }
        
        if (rp == NULL)  // No address succeeded 
        {
            fprintf(stderr, "Could not connect\n");
            exit(EXIT_FAILURE);
        }
        
        pid_t pid;
        int fd[2];
        pipe(fd);
        pid = fork();
        if(pid == 0)
        {
            dup2(fd[1],1);
            execlp("rec", "rec","-p", NULL);
            perror("exec");
            _exit(127);
        }
        printf("descriptor %d\n",fd[1]);
        for(;;)
        {
            for(i = 0; i < msize; i++)
            {
                read(fd[0], buff, 1);
                *(voice + i) = *buff;
            }
            printf("sending voip %d\n", n++);
            fflush(stdout);
            send(rsfd, voice, msize, 0);
        }

    }
    
    return 0;
}


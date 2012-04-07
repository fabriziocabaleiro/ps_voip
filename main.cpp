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
#include <pthread.h>


struct ptData
{
    int  sfd;
    int testing;
    int rdy;
    int  msize;
};

struct parameters
{
    char *lport;
    char *rport;
    char *addr;
    int  testing;
    int  msize;
};

void *localServer(void *arg);
void decode(struct parameters *, char**, int);
void help();

int main(int argc, char** argv)
{
    pthread_t pt;                                 // Thread for Server         
    FILE *pfd;                                    //
    struct ptData *ptdata = (struct ptData *)malloc(sizeof(struct ptData));
    struct parameters *ps = (struct parameters*)malloc(sizeof(struct parameters));
    
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int rsfd;                                     // remote socket file descriptor
    int lsfd;                                     // local socket file descriptor
    int s;
    int n = 0;
    
    decode(ps, argv, argc);                       // decode input parameters from command line
    ptdata->testing = ps->testing;
    ptdata->msize = ps->msize;
    ptdata->rdy   = 0;
    char *voice = (char*)malloc(ps->msize * sizeof(char));
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family    = AF_UNSPEC;               // Allow IPv4 or IPv6 
    hints.ai_socktype  = SOCK_DGRAM;              // UDP 
    hints.ai_flags     = AI_PASSIVE;              // For wildcard IP address   
    hints.ai_protocol  = 0;                       // Any protocol
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;
    
                                                  // Setting up Server 
    s = getaddrinfo(NULL, ps->lport, &hints, &result);
    if (s != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

                                                  // get socket and bind it
    for (rp = result; rp != NULL; rp = rp->ai_next) 
    {
        lsfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lsfd == -1)
            continue;
        if (bind(lsfd, rp->ai_addr, rp->ai_addrlen) == 0)        // bind ready
            break;                                              
        close(lsfd);
    }
    
    if (rp == NULL) {                             // No address succeeded 
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(result);
    ptdata->sfd = lsfd;
    pthread_create(&pt, NULL, localServer, (void*)ptdata);   
    
    while(! ptdata->rdy)
        usleep(1000);
    for(;;)
    {
        if(ps->addr == NULL)
        {
            printf("IP Address: ");
            scanf("%s",ps->addr);
        }
        if(ps->rport == NULL)
        {
            printf("Port Server: ");
            scanf("%s",ps->rport);
        }
        
        s = getaddrinfo(ps->addr, ps->rport, &hints, &result);
        
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
        
        pfd = popen("arecord -c 1 -f S16_LE -r 48000 -F 30000 --period-size=6 --buffer-size=240 | ./encoder","r");

        for(;;)
        {
            fread(voice, sizeof(char), ps->msize, pfd);
            if(ps->testing)
            {
                printf("sending voip %d\n", n++);
                fflush(stdout);
            }
            send(rsfd, voice, ps->msize, 0);
        }

    }
    
    return 0;
}

// Function to decode input parameters
void decode(struct parameters *param, char **argv, int argc)
{
    int i;
    param->addr = NULL;
    param->lport = (char*)"6661";
    param->rport = NULL;
    param->testing = 0;
    param->msize = 500;
    for(i = 1; i < argc; i++)
    {
        if(!strcmp(*(argv + i), "-lp"))
            param->lport = *(argv + i + 1);
        
        if(!strcmp(*(argv + i), "-rp"))
            param->rport = *(argv + i + 1);
        
        if(!strcmp(*(argv + i), "-a"))
            param->addr = *(argv + i + 1);
        
        if(!strcmp(*(argv + i), "-t"))
            param->testing = 1;
        
        if(!strcmp(*(argv + i), "-s"))
            param->msize = atoi(*(argv + i + 1));
        
        if(!strcmp(*(argv + i), "-h"))
            help();
    }
}

// Local server for receiving audio
void * localServer(void *arg)
{
    struct ptData *ptdata = (struct ptData*)arg;
    int n = 0;
    FILE *pfd;
    pfd = popen("./decoder | aplay -c 1 -f S16_LE -r 48000 -F 30000 --period-size=6 --buffer-size=240","w");
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    int nread;
    char *buf = (char*)malloc(ptdata->msize * sizeof(char));
    
    printf("VoIP Server Running\n");
    ptdata->rdy = 1;
    for(;;)
    {
        peer_addr_len = sizeof(struct sockaddr_storage);
        nread = recvfrom(ptdata->sfd, buf, ptdata->msize, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
        if(nread == -1)
            continue;
        fwrite(buf, sizeof(char), ptdata->msize, pfd);
        if(ptdata->testing)
        {
            printf("Receiving message %d\n",++n);
            fflush(stdout);
        }
    }
}

// Help
void help()
{
    printf("OPTIONS\n");
    printf("\t-lp  local port\n");
    printf("\t-rp  remote port\n");
    printf("\t-a   remote address\n");
    printf("\t-s   msize\n");
    printf("\t-t   running testing mode\n");
    printf("\t-h   display this help\n");
    exit(EXIT_SUCCESS);
}
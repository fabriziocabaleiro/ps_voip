/* 
 * File:   main.cpp
 * Author: Fabrizio Cabaleiro
 * VoIP, system programming project
 * 
 * Created on March 24, 2012, 4:08 PM
 */

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
    char *prot;     // protocol
    char *br;       // sample bit rate
    int  sfd;       // socket file descriptor for server
    int  *wait;     // wait option to wait the other person to set his server
    int  testing;   // print info for testing
    int  rdy;       // true when server is ready
    int  msize;     // size of data to be sent
    char *audio;    // set audio program
};

struct parameters
{
    char *prot;     // protocol
    char *br;       // sample bit rate
    char *audio;    // set audio program
    char *lport;    // local port
    char *rport;    // remote port
    char *addr;     // address
    int  testing;   // print info for testing
    int  msize;     // size of data to be sent
    int  wait;      // wait option to wait the other person to set his server
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
    char *cmd = (char*)malloc(200 * sizeof(char));
    int rsfd;                                     // remote socket file descriptor
    int lsfd;                                     // local socket file descriptor
    int s;
    int n = 0;
    
    decode(ps, argv, argc);                       // decode input parameters from command line
    ptdata->testing = ps->testing;
    ptdata->msize = ps->msize;
    ptdata->wait  = &ps->wait;
    ptdata->rdy   = 0;
    ptdata->audio = ps->audio;
    ptdata->br    = ps->br;
    ptdata->prot  = ps->prot;
    
    char *voice = (char*)malloc(ps->msize * sizeof(char));
       
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family    = AF_UNSPEC;               // Allow IPv4 or IPv6 
    if(!strcmp(ps->prot,"udp"))
        hints.ai_socktype  = SOCK_DGRAM;          // UDP 
    else
        hints.ai_socktype  = SOCK_STREAM;         // TCP 
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
    
    if(!strcmp(ps->prot,"udp")) // if using UDP
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
        while(ps->wait)
            usleep(100000);
        
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
        
        if(! strcmp(ps->audio,"alsa"))
            sprintf(cmd,"arecord -t raw -c 1 -f S16_LE -r 8000 -B 30000 | ./encoder/encoder",ps->br);
        else if(! strcmp(ps->audio,"sox"))
            sprintf(cmd,"rec -r %s -q -p",ps->br);
        pfd = popen(cmd,"r");

        for(;;)
        {
            fread(voice, sizeof(char), ps->msize, pfd);
            if(ps->testing)
            {
                printf("sending voip %d\n", n++);
                fflush(stdout);
            }
            fflush(pfd);
            send(rsfd, voice, ps->msize, 0);
        }

    }
    
    return 0;
}

// Function to decode input parameters
void decode(struct parameters *param, char **argv, int argc)
{
    int i;
    param->wait  = 0;
    param->audio = (char*)"alsa";
    param->prot  = (char*)"udp";
    param->br    = (char*)"10000";
    param->addr  = NULL;
    param->lport = (char*)"6661";
    param->rport = NULL;
    param->msize = 500;
    param->testing = 0;
    for(i = 1; i < argc; i++)
    {
        if(!strcmp(*(argv + i), "-lp"))
            param->lport = *(argv + i + 1);
        
        if(!strcmp(*(argv + i), "-rp"))
            param->rport = *(argv + i + 1);
        
        if(!strcmp(*(argv + i), "-ad"))
            param->addr = *(argv + i + 1);
        
        if(!strcmp(*(argv + i), "-r"))
            param->br = *(argv + i + 1);
        
        if(!strcmp(*(argv + i), "-p"))
            param->prot = *(argv + i + 1);
        
        if(!strcmp(*(argv + i), "-t"))
            param->testing = 1;
        
        if(!strcmp(*(argv + i), "-w"))
            param->wait = 1;
        
        if(!strcmp(*(argv + i), "-s"))
            param->msize = atoi(*(argv + i + 1));
        
        if(!strcmp(*(argv + i), "-au"))
            param->audio = *(argv + i + 1);
        
        if(!strcmp(*(argv + i), "-h"))
            help();
    }
}

// Local server for receiving audio
void * localServer(void *arg)
{
    char *cmd = (char*)malloc(200 * sizeof(char));
    struct ptData *ptdata = (struct ptData*)arg;
    int n = 0;
    int first = 1;
    int session_fd;
    FILE *pfd;
    if(! strcmp(ptdata->audio,"alsa"))
        sprintf(cmd,"./decoder/decoder | aplay -c 1 -f S16_LE -r 8000 -B 30000",ptdata->br);
    else if(! strcmp(ptdata->audio,"sox"))
        sprintf(cmd,"play -q -r %s -",ptdata->br);
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    int nread;
    char *buf = (char*)malloc(ptdata->msize * sizeof(char));
    
    printf("VoIP Server Running\n");
    ptdata->rdy = 1;
    if(!strcmp(ptdata->prot,"udp"))
    {
        for(;;)
        {
            peer_addr_len = sizeof(struct sockaddr_storage);
            nread = recvfrom(ptdata->sfd, buf, ptdata->msize, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
            if(nread == -1)
                continue;
            if(first)
            {
                pfd = popen(cmd,"w");
                first = 0;
            }
            fwrite(buf, sizeof(char), ptdata->msize, pfd);
            if(ptdata->testing)
            {
                printf("Receiving message %d\n",++n);
                fflush(stdout);
            }
            fflush(pfd);
            if(*ptdata->wait)
                *ptdata->wait = 0;
        }
    }
    else
    {
        listen(ptdata->sfd, 5);
        session_fd = accept(ptdata->sfd, 0, 0);
        if(session_fd == -1)
            printf("Error trying to accept connection\n");
        else
        {
            printf("A new VoIP chat has started\n");
            while((n = recv(session_fd, buf, ptdata->msize, 0)) > 0)
            {
                if(first)
                {
                    pfd = popen(cmd,"w");
                    first = 0;
                }
                fwrite(buf, sizeof(char), ptdata->msize, pfd);
            }
            printf("VoIP chat finished\n");

        }
    }
}

// Help
void help()
{
    printf("OPTIONS\n");
    printf("\t-lp    local port\n");
    printf("\t-rp    remote port\n");
    printf("\t-ad    remote address\n");
    printf("\t-s     msize, this determinate the package size to be sent through internet\n");
    printf("\t-w     wait to the other person to start his server\n");
    printf("\t-au    select between alsa or sox\n");
    printf("\t-r     set sample bit rate\n");
    printf("\t-p     set communication protocol tcp/udp\n");
    printf("\t-t     running testing mode\n");
    printf("\t-h     display this help\n");
    printf("\n");
    printf("EXAMPLES\n");
    printf("\t$ voip -lp <local_port> -s 100 -au sox\n");
    printf("\tThis will use local_port to leasing, it will send and receive frame of 100 bytes\nand use sox to play and record audio\n\n");
    printf("\t$ voip -lp <local_port> -s 100 -au alsa -rp <remote_port> -ad <address>\n");
    printf("\tThis is similar to the first one, but it is using alsa for recording and playing audio\nand it sends data to the address:remote_port\n");
    printf("");
    exit(EXIT_SUCCESS);
}

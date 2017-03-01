#define _GNU_SOURCE

#include "types.h"
#include "utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

void usage(){
    printf("Invalid arguments!\n");
    printf("Syntax: ./bankclient "
           "-h <server_host> "
           "-p <port> "
           "-i <command_file>\n");
}

int main(int argc, char *argv[]){
    if(argc != 7){
        usage();
    }
    /* Parsing command line arguments. */
    int port = 0;
    char host[300] = "";
    char file_name[300] = "";
    for(int i = 1; i < argc; i = i + 2){
        if(streq(argv[i], "-h"))
            strcpy(host, argv[i + 1]);
        else if(streq(argv[i], "-p"))
            port = atoi(argv[i + 1]);
        else if(streq(argv[i], "-i"))
            strcpy(file_name, argv[i + 1]);
    }
    if(!port || streq(host,"") || streq(file_name, "")){
        usage();
        return -1;
    }
    FILE *f = fopen(file_name, "r");
    bool read_from_file = TRUE;
    if(f == NULL){
       printf("Could not read from file \n");
       read_from_file = FALSE;
    }
    int sock;
    unsigned int serverlen;
    struct sockaddr_in server;
    struct sockaddr *serverptr;
    struct hostent *rem;
    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("Client socket creation failed");
        exit(1);
    }
    if((rem = gethostbyname(host)) == NULL){
        perror("gethostbyname by client failed");
        exit(1);
        
    }
    server.sin_family = PF_INET;
    bcopy((char *) rem->h_addr, (char *) &server.sin_addr, rem->h_length);
    server.sin_port = htons(port);
    serverptr = (struct sockaddr *) &server;
    serverlen = sizeof(server);
    if(connect(sock, serverptr, serverlen) < 0){
        perror("Connection request by client failed");
        exit(1);
    }
    printf("Client requested connection to host %s port %d\n",
           host, port);
    // char mybuf[1000];
    char *mybuf = NULL;
    size_t len = 0;
    ssize_t b_read;
    char size_buf[10];
    char *msg;
    int real_size;
    while(1){
        /*
        if(!fgets(mybuf, sizeof(mybuf), stdin)){
            continue;
            } */
        if(read_from_file){
            if((b_read = getline(&mybuf, &len, f)) == -1){
                read_from_file = FALSE;
                fclose(f);
                continue;
            }
        }
        else{
            printf("%% "); /* Give user a prompt */
            if((b_read = getline(&mybuf, &len, stdin)) == -1){
                continue;
            }
        }

        
        mybuf[strcspn(mybuf, "\n")] = '\0'; /* Remove newline */

        /* Check if command is local */
        if(streq(mybuf, "exit")){
            break;
        }
        else if(strncmp(mybuf,"sleep",strlen("sleep")) == 0){
            int ms;
            sscanf(mybuf, "%*s %d", &ms);
            usleep(ms * 1000);
            continue;
        }
        
        /* Inform server of real cmd size */
        real_size = strlen(mybuf) + 1;
        sprintf(size_buf, "%09d", real_size);
        printf("\n");
        printf("Sending: %s\n", mybuf);
        if(write(sock, size_buf, 10) < 0){
            perror("Could not write to socket");
            exit(1);
        }
        /* Send actual message */
        if(write(sock, mybuf, real_size) < 0){
            perror("Could not write to socket");
            exit(1);
        }
        printf("----------\n");
        /* Read server's response size */
        if(!safe_read(sock, size_buf, 10)){
            perror("Client could not read");
            exit(1);
        }
        real_size = atoi(size_buf);
        msg = malloc(sizeof(char) * real_size);
        if(!safe_read(sock, msg, real_size)){
            perror("Client could not read from socket");
            exit(1);
        }
        printf("Server Response: %s \n", msg);
        free(msg);
    }
    if(mybuf)
        free(mybuf);
    close(sock);
}

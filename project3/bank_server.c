#include "bankrecord.h"
#include "bank_server.h"
#include "hash.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 100
#define HASH_BUCKETS 1009
#define MAX_SOCKETS 1000
#define HASH_BPL 10
#define HASH_LOCKS ((HASH_BUCKETS % HASH_BPL == 0) \
                    ? HASH_BUCKETS / HASH_BPL : \
                    HASH_BUCKETS / HASH_BPL + 1)

/* Socket queue with jobs coming from clients */
int socket_queue[MAX_SOCKETS];
int socket_queue_size = 0;
pthread_mutex_t socket_queue_mtx;
pthread_cond_t socket_queue_cond_empty;

/* Hash with bank records */
hashptr records;
pthread_mutex_t records_locks[HASH_LOCKS]; /* One lock per HASH_BPL buckets. */

int port = 0;
int max_queue_size = 0;
int thread_pool_size = 0;

/* Socket related info. */
int sock;
struct sockaddr_in client;
unsigned int clientlen;
struct sockaddr *clientptr;
struct hostent *rem;

void usage(){
    printf("Invalid arguments!\n");
    printf("Syntax: ./bankserver "
           "-p <port> "
           "-s <thread_pool_size> "
           "-q <queue_size>\n");
}

int main(int argc, char *argv[]){
    if(argc != 7){
        usage();
    }
    /* Parsing command line arguments. */
    for(int i = 1; i < argc; i = i + 2){
        if(streq(argv[i], "-p"))
            port = atoi(argv[i + 1]);
        else if(streq(argv[i], "-s"))
            thread_pool_size = atoi(argv[i + 1]);
        else if(streq(argv[i], "-q"))
            max_queue_size = atoi(argv[i + 1]);       
    }
    if(!port || !thread_pool_size || !max_queue_size){
        usage();
        return -1;
    }

    printf("Server initialized with: \n");
    printf("Port: %d \n", port);
    printf("Thread pool size: %d\n", thread_pool_size);
    printf("Queue size: %d\n", max_queue_size);
    
    /* Preventing server from crashing when client is killed. */
    signal(SIGPIPE, SIG_IGN);
    
    /* Creating hash with bank records. */
    records = hashcreate(HASH_BUCKETS);

    /* Initializing mutexes and cond vars */
    pthread_mutex_init(&socket_queue_mtx, 0);
    pthread_cond_init(&socket_queue_cond_empty, 0);
    for(int i = 0; i < HASH_LOCKS; i++){
        pthread_mutex_init(&(records_locks[i]), 0);
    }

    /* Creating our thread pool */
    pthread_t workers[thread_pool_size];
    for(int i = 0; i < thread_pool_size; i++){
        pthread_create(&(workers[i]), 0, worker_job, 0);
    }

    /* Initializing sockets */
    struct sockaddr_in server;
    unsigned int serverlen;
    struct sockaddr *serverptr;
    
    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket creation failed");
        exit(1);
    }
    server.sin_family = PF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    serverptr = (struct sockaddr *) &server;
    serverlen = sizeof(server);
    if(bind(sock, serverptr, serverlen) < 0){
        perror("Could not bind socket to address");
        exit(1);
    }
    if(listen(sock, BACKLOG) < 0){
        perror("Listening for connections failed");
        exit(1);
    }
    clientptr = (struct sockaddr *) &client;
    clientlen = sizeof(client); // TODO: check this here
    printf("Server running on port: [%d]\n", port);
    
    /* Creating Master Thread. */
    pthread_t master;
    pthread_create(&master, 0, master_job, 0);
    pthread_join(master, NULL);
}

void * master_job(void * arg){
    int newsock;
    while(1){
        if(socket_queue_size < max_queue_size){
            if ((newsock = accept(sock, clientptr, &clientlen)) < 0){
                perror("Could not accept connection");
                exit(1);          
            }
            if((rem = gethostbyaddr((char *) &client.sin_addr.s_addr,
                                    sizeof(client.sin_addr.s_addr),
                                    client.sin_family)) == NULL){
                perror("gethostbyaddr failed");
                exit(1);            
            }            
            printf("Accepted connection from %s (Socket: %d)\n", rem->h_name, newsock);
            pthread_mutex_lock(&socket_queue_mtx);
            /* Add socket to queue. */
            socket_queue[socket_queue_size] = newsock;
            socket_queue_size++;
            /* Print queue data. */
            //printf("Socket queue [size=%d] is: ", socket_queue_size);
            //for(int i = 0; i < socket_queue_size; i++){
                //printf("%d ", socket_queue[i]);
            //}
            //printf("\n");
            pthread_mutex_unlock(&socket_queue_mtx);
            /* Let threads know. */
            pthread_cond_signal(&socket_queue_cond_empty);
        }
    }
    pthread_exit(0);
}

void * worker_job(void *arg){
    while(1){ /* Loops around till server shuts down */
        pthread_mutex_lock(&socket_queue_mtx);
        //printf("Thread %lu in critical section \n", pthread_self());
        while(socket_queue_size == 0){
            //printf("Thread %lu found queue empty! \n", pthread_self());
            pthread_cond_wait(&socket_queue_cond_empty, &socket_queue_mtx);
        }
        int mysocket = socket_queue[socket_queue_size - 1];
        socket_queue_size--;
        printf("->%lu got job from the queue\n", pthread_self());
        pthread_mutex_unlock(&socket_queue_mtx);
        char *mymsg;
        char size_buf[10];
        int cmd_size;
        char myresponse[1000];
        int response_size;
        struct server_command mycmd;
        while(1){ /* Loops around till client leaves */
            /* Read size of command coming */
            if(!safe_read(mysocket, size_buf, 10)){
                printf("Client terminated connection.\n");
                break;
            }
            cmd_size = atoi(size_buf);
            mymsg = malloc(sizeof(char) * cmd_size);
            if(!safe_read(mysocket, mymsg, cmd_size)){
                printf("Client terminated connection.\n");
                break;
            }
            printf("Command received: %s\n", mymsg);
            mycmd = create_command(mymsg, cmd_size);
            free(mymsg);
            execute_cmd(mycmd, myresponse);
            delete_command(mycmd);
            /* Send size of response to client. */
            response_size = strlen(myresponse) + 1;
            sprintf(size_buf, "%09d", response_size);
            if(write(mysocket, size_buf, 10) < 0){
                printf("Client terminated connection.\n");
                break;
            }
            /* Send response back to client. */
            if(write(mysocket, myresponse, response_size) < 0){
                printf("Client terminated connection.\n");
                break;
            }
        }
        close(mysocket);
    }
    pthread_exit(0);
}

void execute_cmd(struct server_command mycmd, char * response){
    int hash_to_lock1, hash_to_lock2;
    int money;
    bank_record br;
    int delay = 0;
    strcpy(response, ""); /* Clear response */
    if(streq(mycmd.cmd, "add_account")){
        if(mycmd.argc < 2 || mycmd.argc > 3){
            sprintf(response, SYNTAX);
            return ;
        }
        money = atoi(mycmd.argv[0]);
        if(money <= 0){
            sprintf(response, CRT_FAIL, mycmd.argv[1], mycmd.argv[0]);
            return ;
        }
        if(mycmd.argc == 3){
            delay = atoi(mycmd.argv[2]);
        }
        hash_to_lock1 = hash(mycmd.argv[1], HASH_BUCKETS);
        br = bank_record_create(mycmd.argv[1], money); /* Hash type */
        pthread_mutex_lock(&(records_locks[hash_to_lock1/HASH_BPL]));
        /* Check if name exists. */
        bank_record br2;
        bool done;
        br2 = hashsearch(records, mycmd.argv[1]);
        if(br2 != NULL){
            done = FALSE;
        }
        else{
            done = hashinsert(records, mycmd.argv[1], br);
        }
        if(done){
            sprintf(response, CRT_DONE, br->name, br->amount);
        }
        else{
            sprintf(response, CRT_FAIL, mycmd.argv[1], mycmd.argv[0]);
        }
        usleep(delay * 1000); /* We need ms. */
        pthread_mutex_unlock(&(records_locks[hash_to_lock1/HASH_BPL]));
    }
    else if(streq(mycmd.cmd, "print_balance")){
        if(mycmd.argc < 1 && mycmd.argc > 2){
            sprintf(response, SYNTAX);
            return ;
        }
        if(mycmd.argc == 2){
            delay = atoi(mycmd.argv[1]);
        }
        hash_to_lock1 = hash(mycmd.argv[0], HASH_BUCKETS);
        pthread_mutex_lock(&(records_locks[hash_to_lock1/HASH_BPL]));
        br = hashsearch(records, mycmd.argv[0]);
        usleep(delay * 1000);
        pthread_mutex_unlock(&(records_locks[hash_to_lock1/HASH_BPL]));
        if(br == NULL){
            sprintf(response, PRINT_FAIL, mycmd.argv[0]);
        }        
        else{
            sprintf(response, PRINT_DONE, br->name, br->amount);
        }
    }
    else if(streq(mycmd.cmd, "add_transfer")){
        if(mycmd.argc < 3 || mycmd.argc > 4){
            sprintf(response, SYNTAX);
            return ;
        }
        if(mycmd.argc == 4){
            delay = atoi(mycmd.argv[3]);
        }
        
        money = atoi(mycmd.argv[0]);
        if(money <= 0){
            sprintf(response, TRANSFER_FAIL,
                    mycmd.argv[1], mycmd.argv[2],
                    mycmd.argv[0], delay);
            return ;
        }
        /* Determine which locks to get. */
        bool same_hash = FALSE;
        int temp;
        hash_to_lock1 = hash(mycmd.argv[1], HASH_BUCKETS) / HASH_BPL;
        hash_to_lock2 = hash(mycmd.argv[2], HASH_BUCKETS) / HASH_BPL;
        if(hash_to_lock1 > hash_to_lock2){
            temp = hash_to_lock1;
            hash_to_lock1 = hash_to_lock2;
            hash_to_lock2 = temp;
        }
        else if(hash_to_lock1 == hash_to_lock2){
            same_hash = TRUE;
        }
        if(same_hash){
            pthread_mutex_lock(&(records_locks[hash_to_lock1]));
        }
        else{
            pthread_mutex_lock(&(records_locks[hash_to_lock1]));
            pthread_mutex_lock(&(records_locks[hash_to_lock2]));
        }
        /* Search source and destination */
        bank_record br2;
        bool done;
        br = hashsearch(records, mycmd.argv[1]);
        br2 = hashsearch(records, mycmd.argv[2]);
        if(br == NULL || br2 == NULL){
            /* One of two does not exist. */
            done = FALSE;
        }
        else{
            /* Transfer amount from source to destination. */
            int source_total = bank_record_amount(br);
            int dest_total = bank_record_amount(br2);
            if(source_total - money < 0){
                /* Cant give more than source owns. */
                done = FALSE;
            }
            else{
                source_total -= money;
                dest_total += money;
                bank_record_update(br, source_total);
                bank_record_update(br2, dest_total);
                bank_record_append(br2, br->name, money);
                done = TRUE;
            }
        }
        if(same_hash){
            pthread_mutex_unlock(&(records_locks[hash_to_lock1]));
        }
        else{
            pthread_mutex_unlock(&(records_locks[hash_to_lock1]));
            pthread_mutex_unlock(&(records_locks[hash_to_lock2]));
        }
        if(done){
            sprintf(response, TRANSFER_DONE,
                    mycmd.argv[1], mycmd.argv[2],
                    mycmd.argv[0], delay);
        }
        else{
            sprintf(response, TRANSFER_FAIL,
                    mycmd.argv[1], mycmd.argv[2],
                    mycmd.argv[0], delay);
        }
    }
    else if(streq(mycmd.cmd, "print_multi_balance")){
        if(mycmd.argc == 0){
            sprintf(response, SYNTAX);
            return ;
        }
        /* Check if we have delay (if last arg is number). */
        bool last_is_delay = FALSE;
        if(is_number(mycmd.argv[mycmd.argc - 1])){
            delay = atoi(mycmd.argv[mycmd.argc - 1]);
            last_is_delay = TRUE;
        }
        int lock_arr_size;
        if(last_is_delay)
            lock_arr_size = mycmd.argc - 1;
        else
            lock_arr_size = mycmd.argc;
            
        int hashes_to_lock[lock_arr_size];
        int htl_index = 0;
        int current_lock;
        bool exists = FALSE;
        for(int i = 0; i < lock_arr_size; i++){
            current_lock = hash(mycmd.argv[i], HASH_BUCKETS) / HASH_BPL;
            for(int j = 0; j < htl_index; j++)
                if(hashes_to_lock[j] == current_lock)
                    exists = TRUE;
            if(!exists)
                hashes_to_lock[htl_index++] = current_lock;
        }
        /* Sort locks to avoid deadlocks. */
        qsort(hashes_to_lock, htl_index, sizeof(int), int_cmp);
        for(int i = 0; i < htl_index; i++){
            pthread_mutex_lock(&records_locks[hashes_to_lock[i]]);
        }
        bool failed = FALSE;
        char print_buffer[900];
        strcpy(print_buffer, "(");
        for(int i = 0; i < lock_arr_size; i++){
            br = hashsearch(records, mycmd.argv[i]);
            if(br == NULL){
                failed = TRUE;
                break;
            }
            else{
                sprintf(print_buffer, "%s%s/%d:", print_buffer,
                        br->name, br->amount);
            }
        }
        if(failed){
            strcpy(print_buffer, "(");
            for(int i = 0; i < lock_arr_size; i++){
                sprintf(print_buffer, "%s%s:", print_buffer,
                        mycmd.argv[i]);
            }
            sprintf(print_buffer, "%s)", print_buffer);
            sprintf(response, PRINT_MULTI_FAIL, print_buffer);
        }
        else{
            sprintf(print_buffer, "%s)", print_buffer);
            sprintf(response, PRINT_MULTI_DONE, print_buffer);
        }
        /* Delay. */
        usleep(delay * 1000);

        /* Unlock in reverse order. */
        for(int i = htl_index - 1; i >= 0; i--){
            pthread_mutex_unlock(&records_locks[hashes_to_lock[i]]);
        }
    }
    else if(streq(mycmd.cmd, "add_multi_transfer")){
        if(mycmd.argc < 3){
            sprintf(response, SYNTAX);
            return ;
        }
        /* Check if we have delay (if last arg is number). */
        bool last_is_delay = FALSE;
        if(is_number(mycmd.argv[mycmd.argc - 1])){
            delay = atoi(mycmd.argv[mycmd.argc - 1]);
            last_is_delay = TRUE;
        }
        int lock_arr_size;
        if(last_is_delay)
            lock_arr_size = mycmd.argc - 2;
        else
            lock_arr_size = mycmd.argc - 1;
            
        int hashes_to_lock[lock_arr_size];
        int htl_index = 0;
        int current_lock;
        bool exists = FALSE;
        for(int i = 0; i < lock_arr_size; i++){
            current_lock = hash(mycmd.argv[i + 1], HASH_BUCKETS) / HASH_BPL;
            for(int j = 0; j < htl_index; j++)
                if(hashes_to_lock[j] == current_lock)
                    exists = TRUE;
            if(!exists)
                hashes_to_lock[htl_index++] = current_lock;
        }
        /* Sort locks to avoid deadlocks. */
        qsort(hashes_to_lock, htl_index, sizeof(int), int_cmp);
        for(int i = 0; i < htl_index; i++){
            pthread_mutex_lock(&records_locks[hashes_to_lock[i]]);
        }
        /* Check if source has sufficient funds. */
        int source_money;
        int dest_money;
        bool success = TRUE;
        bank_record br2;
        money = atoi(mycmd.argv[0]);
        br = hashsearch(records, mycmd.argv[1]);
        if(br != NULL){
            /* Check if source can afford transfer. */
            source_money = br->amount;
            if(source_money < money * (lock_arr_size - 1)){
                success = FALSE;
            }
            else{
                /* Check if all destinations exist. */
                for(int i = 2; i < lock_arr_size + 1; i++){
                    br2 = hashsearch(records, mycmd.argv[i]);
                    if(br2 == NULL){
                        success = FALSE;
                        break;
                    }
                }
                /* Transfer amount to all args */
                if(success){
                    for(int i = 2; i < lock_arr_size + 1; i++){
                        br2 = hashsearch(records, mycmd.argv[i]);
                        dest_money = bank_record_amount(br2) + money;
                        bank_record_update(br2, dest_money);
                        bank_record_append(br2, mycmd.argv[1], money);
                    }
                    source_money -= money * (lock_arr_size - 1);
                    br->amount = source_money;
                    bank_record_update(br, source_money);
                }
            }
        }
        else{
            success = FALSE;
        }
        
        /* Delay. */
        usleep(delay * 1000);

        /* Unlock in reverse order. */
        for(int i = 0; i < htl_index; i++){
            pthread_mutex_unlock(&records_locks[hashes_to_lock[i]]);
        }
        
        /* Prepare output. */
        char print_buffer[900];
        strcpy(print_buffer, "(");
        for(int i = 0; i < mycmd.argc; i++){
            sprintf(print_buffer, "%s%s:", print_buffer, mycmd.argv[i]);
        }
        sprintf(print_buffer, "%s)", print_buffer);

        if(success)
            sprintf(response, TRANSFER_MULTI_DONE, print_buffer);
        else
            sprintf(response, TRANSFER_MULTI_FAIL, print_buffer);
    }
    else{
        /* Invalid command. */
        sprintf(response, UKNOWN);
    }
}

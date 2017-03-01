#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "graph.h"
#include "hash.h"
#include "bank.h"
#include "stack.h"

/* Globals */
int BUCKETS; /* number of hash buckets */

void print_usage(char *msg){
    printf("%s\n", msg);
    printf("Correct usage is ./elegxos -b NumberOfBuckets -o opfile.txt\n");
    printf("-o flag is optional.\n");
}

void parse_cmd(struct bank **b, char *cmd){
    const char delimiters[2] = " ";
    char *token;
    token = strtok(cmd, delimiters);
    if(*b == NULL && !streq(token, "createnodes")){
        printf("failure: No bank system exists. Use createnodes. \n");
    }
    else if(streq(token, "createnodes")){
        long id;
        token = strtok(NULL, delimiters); /* Get first parameter */
        while(token != NULL){ /* Get the rest parameters */
            id = strtol(token, NULL, 10);
            if(streq(token, "\n")) ; /* Skip newlines */
            else if(id < 1000000 || id > 9999999){
                printf("failure: ids are 7-digit numbers \n");
            }
            else{
                if(*b == NULL){
                    *b = bankinit(BUCKETS);
                }
                bankcreatenode(*b, (int) id);
                printf("success: created %d \n", (int) id);
            }
            token = strtok(NULL, delimiters);       
        }
    }
    else if(streq(token, "delnodes")){
        long id;
        token = strtok(NULL, delimiters); /* Get first parameter */
        while(token != NULL){ /* Get the rest parameters */
            id = strtol(token, NULL, 10);
            struct node *n = hashsearch((*b)->hashtable, (int) id);
            if(n == NULL){
                printf("failure: node %d does not exist. \n", (int) id);
                return ;
            }
            else{
                if(bankdelnode(*b, (int) id)){
                    printf("success: deleted %d \n", (int) id);
                }
                else{
                    printf("failure: node %d has edges connected. \n", (int) id);
                }
            }
            token = strtok(NULL, delimiters);       
        }
    }
    else if(streq(token, "addtran")){
        long id1, id2;
        amount_t amount;
        struct node *n1, *n2;
        token = strtok(NULL, delimiters); /* First id */
        id1 = strtol(token, NULL, 10);
        token = strtok(NULL, delimiters); /* Second id */
        id2 = strtol(token, NULL, 10);
        token = strtok(NULL, delimiters); /* Amount */
        amount = strtod(token, NULL);
        n1 = hashsearch((*b)->hashtable, (int) id1);
        n2 = hashsearch((*b)->hashtable, (int) id2);
        if(n1 == NULL){
            printf("failure: %d does not exist. \n", (int) id1);
        }
        else if(n2 == NULL){
            printf("failure: %d does not exist. \n", (int) id2);
        }
        else{
            /* Add transaction */
            bankaddtran(*b, id1, id2, amount);
            printf("success: added transaction %d %d with %f \n",
                   (int) id1, (int) id2, amount);
        }
    }
    else if(streq(token, "deltran")){
        long id1, id2;
        //amount_t amount;
        struct node *n1, *n2;
        token = strtok(NULL, delimiters); /* First id */
        id1 = strtol(token, NULL, 10);
        token = strtok(NULL, delimiters); /* Second id */
        id2 = strtol(token, NULL, 10);
        n1 = hashsearch((*b)->hashtable, (int) id1);
        n2 = hashsearch((*b)->hashtable, (int) id2);
        if(n1 == NULL){
            printf("failure: %d does not exist. \n", (int) id1);
        }
        else if(n2 == NULL){
            printf("failure: %d does not exist. \n", (int) id2);
        }
        else{
            /* Delete transaction */
            if(bankdeltran(*b, id1, id2)){
                printf("success: deleted transaction %d %d \n",
                       (int) id1, (int) id2);
            }
            else{
                printf("failure: transaction does not exist. \n");
            }
        }
    }
    else if(streq(token, "lookup")){
        LOOKUP_TYPE t;
        token = strtok(NULL, delimiters); /* lookup type */
        if(streq(token, "in")) t = LOOK_IN;
        else if(streq(token, "out")) t = LOOK_OUT;
        else t = LOOK_SUM;
        long id;
        struct node *n1;
        token = strtok(NULL, delimiters); /* id */
        id = strtol(token, NULL, 10);
        n1 = hashsearch((*b)->hashtable, (int) id);
        if(n1 == NULL){
            printf("failure: node %d does not exist. \n", (int) id);
        }
        else{
            amount_t amount = banklookup(*b, t, id);
            printf("sucess: ");
            if(t == LOOK_IN) printf("in(%d)", (int) id);
            else if(t == LOOK_OUT) printf("out(%d)", (int) id);
            else printf("sum(%d)", (int) id);
            printf(" = %f \n", amount);
        }
    }
    else if(streq(token, "triangle")){
        long id;
        double amount;
        struct node *n;
        token = strtok(NULL, delimiters); /* node id */
        id = strtol(token, NULL, 10);
        token = strtok(NULL, delimiters); /* minumum amount */
        amount = strtod(token, NULL);
        n = hashsearch((*b)->hashtable, (int) id);
        if(n == NULL){
            printf("failure: node %d does not exist. \n", (int) id);
        }
        else if(amount < 0){
            printf("failure: amount can't be a negative number. \n");
        }
        else{
            bool triangle_exists = FALSE;
            printf("success: triangle(%d, %f) = \n", (int) id, amount);
            triangle_exists = banktriangle(*b, (int) id, amount);
            if(!triangle_exists){
                printf("no triangle with %d \n", (int) id);
            }
        }
    }
    else if(streq(token, "conn")){
        long id1, id2;
        struct node *n1, *n2;
        token = strtok(NULL, delimiters); /* id1 */
        id1 = strtol(token, NULL, 10);
        token = strtok(NULL, delimiters); /* id2 */
        id2 = strtol(token, NULL, 10);
        n1 = hashsearch((*b)->hashtable, (int) id1);
        n2 = hashsearch((*b)->hashtable, (int) id2);
        if(n1 == NULL){
            printf("failure: node %d does not exist. \n", (int) id1);
        }
        else if(n2 == NULL){
            printf("failure: node %d does not exist. \n", (int) id2);
        }
        else{
            printf("success: ");
            bankconn(*b, (int) id1, (int) id2);
        }
    }
    else if(streq(token, "allcycles")){
        long id;
        struct node *n;
        token = strtok(NULL, delimiters); /* node id */
        id = strtol(token, NULL, 10);
        n = hashsearch((*b)->hashtable, id);
        if(n == NULL){
            printf("failure: node %d does not exist. \n", (int) id);
        }
        else{
            printf("success: ");
            bankallcycles(*b, (int) id);
        }
    }
    else if(streq(token, "traceflow")){
        long id;
        long depth;
        struct node *n;
        token = strtok(NULL, delimiters); /* node id */
        id = strtol(token, NULL, 10);
        token = strtok(NULL, delimiters); /* depth */
        depth = strtol(token, NULL, 10);
        n = hashsearch((*b)->hashtable, id);
        if(n == NULL){
            printf("failure: node %d does not exist. \n", (int) id);
        }
        else{
            printf("success: traceflow(%d,%d) = \n", (int) id, (int) depth);
            banktraceflow(*b, (int) id, (int) depth);
        }
    }
    else if(streq(cmd, "bye\n")){
        bankdestroy(*b);
        *b = NULL;
        printf("success: Cleaned memory \n");
    }
    else if(streq(cmd, "print\n")){
        //hash_debug_printall(b->hashtable);
        bankprint(*b);
    }
    else if(streq(token, "dump")){
        token = strtok(NULL, delimiters);
        char filename[MAX_FILE_NAME];
        sprintf(filename, "%s", token);
        FILE *f = fopen(filename, "w");
        if(f == NULL){
            printf("failure: error creating dumpfile. \n");
        }
        else{
            bankdump(*b, f);
            printf("success: dumped to %s \n", filename);
        }
    }
    else if(streq(token, "\n")) ; /* Skip newlines */
    else{
        printf("failure: invalid command: %s \n", cmd);
    }
}

int main(int argc, char *argv[]){
    /* Parsing command line arguments */
    int number_of_buckets;
    char operation_file[MAX_FILE_NAME];
    bool load_opfile = FALSE;
    if(argc != 3 && argc != 5){
        print_usage("Incorrect number of arguments \n");
        exit(-1);
    }
    else if(argc == 3){
        /* Only bucket entries number */
        if(strcmp(argv[1], "-b") != 0){
            print_usage("Bucket entries number is required.");
            exit(-1);
        }
        number_of_buckets = atoi(argv[2]);
    }
    else{
        /* Bucket entries and opfile */
        load_opfile = TRUE;
        if(!strcmp(argv[1], "-b") && !strcmp(argv[3], "-o")){
            number_of_buckets = atoi(argv[2]);
            strncpy(operation_file, argv[4], MAX_FILE_NAME);
        }
        else if(!strcmp(argv[1], "-o") && !strcmp(argv[3], "-b")){
            number_of_buckets = atoi(argv[4]);
            strncpy(operation_file, argv[2], MAX_FILE_NAME);
        }
        else{
            print_usage("Incorrect argument format.");
            exit(-1);
        }
    }

    /* Check for negative buckets */
    if(number_of_buckets < 1){
        print_usage("Bucket entries is a number > 0.");
        exit(-1);
    }

    BUCKETS = number_of_buckets;
    
    /* Create bank system */
    struct bank *b = bankinit(number_of_buckets);

    /* Parse operations file */
    char cmd[MAX_CMD];
    char *line = NULL;
    size_t len = 0;
    ssize_t read; 
    if(load_opfile){
        FILE *f = fopen(operation_file, "r");
        if(f != NULL){
            /* File exists, parse commands line by line */
            while((read = getline(&line, &len, f)) != -1){
                strncpy(cmd, line, MAX_CMD);
                //printf("%s", cmd);
                parse_cmd(&b, cmd);
            }
            fclose(f);
        }
        else{
            printf("File specified does not exist. \n");
        }
    }

    /* Create a prompt */
    while(1){
        printf("%% ");
        read = getline(&line, &len, stdin);
        if(read == -1) break;
        strncpy(cmd, line, MAX_CMD);
        parse_cmd(&b, cmd);
    }
    
    /* Free memory */
    if(b != NULL){
        bankdestroy(b);
    }
    free(line);
}

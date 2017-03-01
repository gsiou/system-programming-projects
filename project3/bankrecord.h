#ifndef BANKRECORD_H
#define BANKRECORD_H
#define MAX_NAME 200

#include "types.h"

struct list_node{
    char name[MAX_NAME];
    int amount;
    struct list_node *next;
};

typedef struct list_node* list_node_ptr;

struct bank_record{
    char name[MAX_NAME];
    int amount;
    int received_size;
    list_node_ptr received;
};

typedef struct bank_record* bank_record;

/* Allocates memory for a bank record and initializes it
 * to values given */
bank_record bank_record_create(char *name, int amount);

/* Returns record's amount. */
int bank_record_amount(bank_record br);

/* Updates record's money with amount. */
void bank_record_update(bank_record br, int newamount);

/* Adds name and amount to list of received amounts. */
void bank_record_append(bank_record br, char *name, int amount);
#endif

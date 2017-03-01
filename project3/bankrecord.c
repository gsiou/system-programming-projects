#include "bankrecord.h"
#include <stdlib.h>
#include <string.h>

bank_record bank_record_create(char *name, int amount){
    bank_record b = malloc(sizeof(struct bank_record));
    strcpy(b->name, name);
    b->amount = amount;
    b->received_size = 0;
    b->received = NULL;
    return b;
}

int bank_record_amount(bank_record br){
    return br->amount;
}

void bank_record_update(bank_record br, int newamount){
    br->amount = newamount;
}

void bank_record_append(bank_record br, char *name, int amount){
    /* First search if name exists alredy in list. */
    list_node_ptr current = br->received;
    bool found = FALSE;
    while(current != NULL){
        if(streq(current->name, name)){
            found = TRUE;
            break;
        }
        current = current->next;
    }
    if(found){
        /* Just update amount. */
        current->amount += amount;
    }
    else{
        /* Create new node to add. */
        list_node_ptr newnode = malloc(sizeof(struct list_node));
        strcpy(newnode->name, name);
        newnode->amount = amount;
        newnode->next = br->received;
        /* Add new node at the beginning of the list. */
        br->received = newnode;
        br->received_size ++;
    }
}

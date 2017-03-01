#include "bank.h"
#include <stdlib.h>
#include <stdio.h>

struct bank * bankinit(int bucket_number){
    struct bank *b = malloc(sizeof(struct bank));
    b->size = 0;
    b->hashtable = hashcreate(bucket_number);
    return b;
}

void bankcreatenode(struct bank *b, nodeid_t id){
    struct node *n = nodecrt(id);
    if(n == NULL){
        perror("failure");
    }
    hashinsert(b->hashtable, id, n);
    b->size++;
}

bool bankdelnode(struct bank *b, nodeid_t id){
    struct node *n = hashsearch(b->hashtable, id);
    if(n != NULL){
        if(nodeclr(n)){
            n = hashdelete(b->hashtable, id);
            if(n != NULL){
                nodedel(n);
                return 1;
            }
            else{
                return 0;
            }
        }
        else{
            return 0;
        }
    }
    return 0;
}

bool bankaddtran(struct bank *b, nodeid_t from, nodeid_t to, amount_t amount){
    struct node* n1 = hashsearch(b->hashtable, from);
    struct node* n2 = hashsearch(b->hashtable, to);
    if(n1 == NULL || n2 == NULL){
        return FALSE;
    }
    struct edge *e;
    e = nodesrch(n1, to, OUT);
    if(e == NULL){
        e = edgecrt(n1, n2, amount);
        /* Add edge to in/out of each node */
        nodeadde(n1, e, OUT);
        nodeadde(n2, e, IN);
    }
    else{
        /* Update edge */
        edgeupd(e, amount);
    }
    return TRUE;
}

bool bankdeltran(struct bank *b, nodeid_t from, nodeid_t to){
    struct edge *target;
    struct node *n1 = hashsearch(b->hashtable, from);
    struct node *n2 = hashsearch(b->hashtable, to);
    bool found1 = FALSE;
    bool found2 = FALSE;
    if(n1 == NULL || n2 == NULL){
        return FALSE;
    }
    else{
        for(int i = 0; i < n1->out_size; i++){
            if(n1->out[i]->to->id == to){
                found1 = TRUE;
                target = n1->out[i];
                nodedele(n1, target, OUT); /* Remove edge from */
                for(int j = 0; j < n2->in_size; j++){
                    if(n2->in[j]->from->id == from){
                        nodedele(n2, target, IN); /* Remove edge to */
                        found2 = TRUE;
                        break;
                    }
                }
                break;
                edgedel(target); /* Destroy edge item itself */
            }
        }
        return found1 && found2;
    }    
}

amount_t banklookup(struct bank *b, LOOKUP_TYPE l, nodeid_t nodeid){
    amount_t amount = 0;
    struct node *n = hashsearch(b->hashtable, nodeid);
    if(n == NULL){
        /* We cant do anything for nonexisting nodes */
        return 0;
    }
    if(l == LOOK_IN || l == LOOK_SUM){
        for(int i = 0; i < n->in_size; i++){
            amount += n->in[i]->amount;
        }
    }
    if(l == LOOK_OUT || l == LOOK_SUM){
        for(int i = 0; i < n->out_size; i++){
            amount -= n->out[i]->amount;
        }
    }
    if(l == LOOK_OUT){
        /* We don't want value to be negative for out */
        return -1 * amount;
    }
    else{
        return amount;
    }
}

bool banktriangle(struct bank *b, nodeid_t nodeid, amount_t k){
    /* First find node (if it exists) */
    struct node *n = hashsearch(b->hashtable, nodeid);
    struct node *temp1, *temp2;
    int jmax, lmax;
    bool triangle = FALSE;
    if(n == NULL){
        /* Node does not exist, return false */
        return FALSE;
    }
    for(int i = 0; i < n->out_size; i++){
        if(n->out[i]->amount >= k){
            temp1 = n->out[i]->to;
            jmax = temp1->out_size;
            for(int j = 0; j < jmax; j++){
                if(temp1->out[j]->amount >= k){
                    temp2 = temp1->out[j]->to;
                    lmax = temp2->out_size;
                    for(int l = 0; l < lmax; l++){
                        if(temp2->out[l]->amount >= k && temp2->out[l]->to->id == nodeid){
                            printf("(%d, %d, %d) \n",
                                   n->id,
                                   temp1->id,
                                   temp2->id
                                );
                            triangle = TRUE;
                        }
                    }
                }
            }
        }
    }
    return triangle;
}

bool bankconn(struct bank *b, nodeid_t from, nodeid_t to){
    struct node *n1 = hashsearch(b->hashtable, from);
    struct node *n2 = hashsearch(b->hashtable, to);
    if(n1 == NULL || n2 == NULL){
        /* Node does not exist, return false */
        return FALSE;
    }
    graphfindpath(n1, n2);
    return TRUE;
}

bool bankallcycles(struct bank *b, nodeid_t nodeid){
    struct node *n1 = hashsearch(b->hashtable, nodeid);
    if(n1 == NULL){
        /* Node does not exist, return false */
        return FALSE;
    }
    graphfindcycles(n1);
    return TRUE;
}

bool banktraceflow(struct bank *b, nodeid_t nodeid, int l){
    struct node *n = hashsearch(b->hashtable, nodeid);
    if(n == NULL){
        /* Node does not exist, return false */
        return FALSE;
    }
    graphfindflow(n, l);
    return TRUE;
}

void bankprint(struct bank *b){
    hashiterate(b->hashtable, 
                (void (*)(struct node *n)) nodeprnt);
}

void bankdump(struct bank *b, FILE *f){
    if(f == NULL){
        return ;
    }
    if(b->size == 0){
        printf("No data to dump.\n");
        return ;
    }
    /* Add all nodes to file */
    struct item *current;
    fprintf(f, "createnodes ");
    for(int i = 0; i < b->hashtable->bucket_number; i++){
        current = b->hashtable->buckets[i].items;
        while(current != NULL){
            fprintf(f, "%d ", current->data->id);
            current = current->next;
        }
    }
    fprintf(f, "\n");

    /* Add all edges to file */
    for(int i = 0; i < b->hashtable->bucket_number; i++){
        current = b->hashtable->buckets[i].items;
        while(current != NULL){
            for(int j = 0; j < current->data->out_size; j++){
                fprintf(f, "addtran %d %d %f \n",
                        current->data->out[j]->from->id,
                        current->data->out[j]->to->id,
                        current->data->out[j]->amount);
            }
            current = current->next;
        }
    }
    fprintf(f, "\n");
    fclose(f);
}

void bankdestroy(struct bank *b){
    /* Delete nodes first */
    hashiterate(b->hashtable,
                (void (*)(struct node *n)) nodedel);
    hashdestroy(b->hashtable);
    free(b);
}

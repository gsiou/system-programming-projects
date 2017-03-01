#include "hash.h"
#include <stdio.h>
#include <stdlib.h>

struct hash* hashcreate(int b){
    struct hash *h = malloc(sizeof(struct hash));
    if(h == NULL){
        return NULL;
    }
    h->bucket_number = b;
    
    /* Allocate space for b buckets */
    h->buckets = malloc(b * sizeof(struct bucket));
    if(h->buckets == NULL){
        return NULL;
    }
    
    /* Initialize buckets */
    int i;
    for(i = 0; i < b; i++){
        h->buckets[i].size = 0;
        h->buckets[i].items = NULL;
    }
    return h;
}

void hashdestroy(struct hash *h){
    struct item *current;
    struct item *temp;
    for(int i = 0; i < h->bucket_number; i++){
        current = h->buckets[i].items;
        while(current != NULL){ /* Free each item */
            temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(h->buckets); /* Free buckets */
    free(h); /* Free hash itself */
}

void hashiterate(struct hash *h, void (*datafunction)(hash_data)){
    struct item *current;
    for(int i = 0; i < h->bucket_number; i++){
        current = h->buckets[i].items;
        while(current != NULL){
            (*datafunction)(current->data);
            current = current->next;
        }
    } 
}

bool hashinsert(struct hash *h, hash_data_id id, hash_data d){
    //hash_data n = nodecrt(id);
    int bucket_index = hash(id, h->bucket_number);
    struct item *new = malloc(sizeof(struct item));
    if(new == NULL){
        return FALSE;
    }
    new->data = d;
    new->next = NULL;
    struct item *current = h->buckets[bucket_index].items;
    if(current == NULL){
        /* Empty bucket */
        h->buckets[bucket_index].items = new;
    }
    else{
        while(current != NULL){
            if(current->next == NULL){
                current->next = new;
                break;
            }
            current = current->next;
        }
    }
    return TRUE;
    
}

struct node* hashsearch(struct hash *h, hash_data_id id){
    int bucket_index = hash(id, h->bucket_number);
    struct item *current = h->buckets[bucket_index].items;
    while(current != NULL){
        if(current->data->id == id){
            return current->data;
        }
        current = current->next;
    }
    /* If nothing is returned till here we dont have the item */
    return NULL;
}

hash_data hashdelete(struct hash *h, hash_data_id id){
    int bucket_index = hash(id, h->bucket_number);
    struct item *current = h->buckets[bucket_index].items;
    struct item *prev = NULL;
    struct item *temp = NULL;
    hash_data data;
    while(current != NULL){
        if(current->data->id == id){
            data = current->data; /* To return it later */
            if(prev == NULL){
                /* We are on the first item */
                temp = current->next;
                free(current); /* Free the item cointainer */
                h->buckets[bucket_index].items = temp;
            }
            else{
                prev->next = current->next;
                free(current); /* Free the item container */                
            }
            return data;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

int hash(hash_data_id id, int bucket_number){
    return id % bucket_number;
}

void hash_debug_printall(struct hash *h){
    struct item *current;
    for(int i = 0; i < h->bucket_number; i++){
        printf("Bucket %d \n", i);
        current = h->buckets[i].items;
        while(current != NULL){
            printf("[%d]: %d(%d) \n", i, current->data->id, current->data->out_size);
            for(int j = 0; j < current->data->out_size; j++){
                printf("(%d)->(%d): %f\n",
                       current->data->out[j]->from->id,
                       current->data->out[j]->to->id,
                       current->data->out[j]->amount);
            }
            current = current->next;
        }
        printf(" \n");
    }
}

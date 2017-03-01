#ifndef HASH_H
#define HASH_H
#include "bankrecord.h"
#include "types.h"

/* Hash content types */
typedef struct bank_record* hash_data;
typedef char* hash_data_id;
/* Data structures */

struct item{
    hash_data data; /* current node */
    struct item *next; /* next item */
};

struct bucket{
    struct item *items; /* array of bucket's items */
    int size; /* number of contents */
};

struct hash{
    struct bucket *buckets; /* array of buckets */
    int bucket_number; /* number of buckets */
};

typedef struct hash* hashptr;

/* Functions */

/* Create a hash and allocate b buckets to it 
 * Returns null if memory allocation fails */
struct hash* hashcreate(int b);

/* Free hash memory and destroy it (does not free data) */
void hashdestroy(struct hash *h);

/* Applies given function to all data */
void hashiterate(struct hash *h, void (*datafunction)(hash_data));

/* Inserts data item to hash (hash by id)  
 * Returns 0 if memory allocation fails; 1 otherwise */
bool hashinsert(struct hash *h, hash_data_id id, hash_data d);

/* Removes data item from hash 
 * Returns that item or null if not found 
 * Does not free item memory */
//hash_data hashdelete(struct hash *h, hash_data_id id);

/* Returns hash data with given id */
hash_data hashsearch(struct hash *h, hash_data_id id);

/* Returns the index of the bucket for the item to be placed */
unsigned int hash(hash_data_id id, int bucket_number);

/* Prints contents of each bucket for debugging */
void hash_debug_printall(struct hash *h);
#endif

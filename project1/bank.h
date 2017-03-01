#ifndef BANK_H
#define BANK_H

#include <stdio.h>
#include "hash.h"
#include "graph.h"
#include "types.h"

/* Data structures */

struct bank{
    struct hash *hashtable;
    int size;
};

typedef enum {LOOK_IN, LOOK_OUT, LOOK_SUM} LOOKUP_TYPE;

/* Functions */

/* Initialize bank system (create hash with bucket_number buckets)*/
struct bank * bankinit(int bucket_number);

/* Create node with id */
void bankcreatenode(struct bank *b, nodeid_t id);

/* Delete nodes with id (only if there are no edges connected) 
 * Returns 1 on success or 0 if there are edges connected or node
 * does not exist */
bool bankdelnode(struct bank *b, nodeid_t id);

/* Creates an edge from one node to another with given amount
 * If edge exits, amount is added.
 * Returns 1 if edge is created 
 * 0 if one of the nodes does not exist */
bool bankaddtran(struct bank *b, nodeid_t from, nodeid_t to, amount_t amount);

/* Deletes edge between nodes
 * Returns 1 if edge is deleted
 * 0 if from or two destination does not exist 
 * or if edge does not exist */
bool bankdeltran(struct bank *b, nodeid_t from, nodeid_t to);

/* Calculates incoming/outcoming/sum amount connected to node 
 * Returns amount requested */
amount_t banklookup(struct bank *b, LOOKUP_TYPE l, nodeid_t nodeid);

/* Checks if node is involved in a triangle with a minimum amount 
 * of k thousand per edge.
 * Returns 1 if node is involved, 0 otherwise */
bool banktriangle(struct bank *b, nodeid_t nodeid, amount_t k);

/* Checks if a path exists between 2 nodes 
 * If it exists, it prints it in human readable form 
 * Returns 0 if nodes do not exist ; 1 otherwise */
bool bankconn(struct bank *b, nodeid_t from, nodeid_t to);

/* Finds and prints all circles that node is involved in.
 * Circles with same suffix are ommited.
 * Each circle contains at least 3 nodes and each node 
 * appears only once in a circle 
 * Retuns 0 if nodes do not exits ; 1 otherwise */
bool bankallcycles(struct bank *b, nodeid_t nodeid);

/* Tracks and prints all the money flow from node 
 * searching in paths with depth l. Each node appears 
 * no more than once. 
 * Returns 0 if node does not exist ; 1 otherwise */
bool banktraceflow(struct bank *b, nodeid_t nodeid, int l);

/* Destroys bank structure freeing all memory
 * used by its data structures */
void bankdestroy(struct bank *b);

/* Prints all the data in human-readable form */
void bankprint(struct bank *b);

/* Dumps current state in file */
void bankdump(struct bank *b, FILE *f);

#endif

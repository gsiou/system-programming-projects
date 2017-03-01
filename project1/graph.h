#ifndef GRAPH_H
#define GRAPH_H

#include "types.h"

/* Data structures */

typedef enum {IN, OUT} DIRECTION;

struct edge{
    amount_t amount; /* amount of transaction */
    struct node *from; /* starting node */
    struct node *to; /* ending node */
    int connected_nodes; /* know when it can be removed */
};

struct node{
    struct edge **out; /* array of pointers to all outgoing edges */
    struct edge **in; /* array of pointers to all incoming edges */
    int out_size; /* size of out array */
    int in_size; /* size of in array */
    int out_max; /* maximum space for out (changing when needed) */
    int in_max; /* maximum space for in (changing when needed) */
    nodeid_t id; /* node id */
    bool visited_flag; /* used for search algorithms */
    bool used_flag; /* used for search algorithms */
};

/* Functions */

/* Creates (allocates memory) for a node and returns it 
 * Returns null if allocation failed */
struct node * nodecrt(nodeid_t id);

/* Deletes (frees memory) of a node 
 * If node has edges, and these edges *
 * are only connected on this end, they are removed */
void nodedel(struct node *n);

/* Returns 0 if node has edges connected, 1 otherwise */
bool nodeclr(struct node *n);

/* Adds edge pointer to out/in edges array */
void nodeadde(struct node *n, struct edge *e, DIRECTION d);

/* Returns pointer to edge if found 
 * NULL if not found */
struct edge* nodesrch(struct node *n, nodeid_t id, DIRECTION d);

/* Deletes edge pointer from out/in edges array */
void nodedele(struct node *n, struct edge *e, DIRECTION d);

/* Prints node and connected out edges */
void nodeprnt(struct node *n);

/* Creates an edge from n1 to n2 with given amount and returns it 
 * Returns null if memory allocation failed */
struct edge * edgecrt(struct node *n1, struct node *n2, amount_t amount);

/* Updates given edge's amount with current amount
 * plus given amount */
void edgeupd(struct edge *e, amount_t plusamount);

/* Deletes an edge */
void edgedel(struct edge * e);

/* Searches for a path between start and end node and prints it */
void graphfindpath(struct node *start_node, struct node *end_node);

/* Searches for cycles that start_node is involved in and prints them */
void graphfindcycles(struct node* start_node);

/* Searches for flow in paths with length l */
void graphfindflow(struct node* start_node, int l); 
#endif

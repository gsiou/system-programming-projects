#include "graph.h"
#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

#define START_OUT 20
#define START_IN 20

struct node * nodecrt(nodeid_t id){
    struct node *n = malloc(sizeof(struct node));
    if(n == NULL){
        return NULL;
    }
    n->id = id;
    /* Initializing edge arrays */
    n->out = malloc(START_OUT * sizeof(struct edge*));
    n->out_max = START_OUT;
    n->out_size = 0;
    n->in = malloc(START_IN * sizeof(struct edge*));
    n->in_max = START_IN;
    n->in_size = 0;
    /* Set visited flag to false, will be changed by search algos */
    n->visited_flag = FALSE;
    n->used_flag = FALSE;
    return n;
}

void nodedel(struct node *n){
    /* Delete out edges */
    for(int i = 0; i < n->out_size; i++){

        /* Only delete edge if other end is disconnected */
        if(n->out[i]->connected_nodes == 2){
            n->out[i]->connected_nodes--;
        }
        else{
            edgedel(n->out[i]);
        }
    }
    
    /* Delete out edges */
    for(int i = 0; i < n->out_size; i++){
        
        /* Only delete edge if other end is disconnected */
        if(n->out[i]->connected_nodes == 2){
            n->out[i]->connected_nodes--;
        }
        else{
            edgedel(n->out[i]);
        }
    }

    /* Free arrays and node itself */
    free(n->out);
    free(n->in);
    free(n);
}

bool nodeclr(struct node *n){
    if(n->out_size == 0 && n->in_size == 0){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

void nodeadde(struct node *n, struct edge *e, DIRECTION d){
    struct edge ***arr_p;
    int *size, *max;
    if(d == IN){
        arr_p = &(n->in);
        size = &(n->in_size);
        max = &(n->in_max);
    }
    else{
        arr_p = &(n->out);
        size = &(n->out_size);
        max = &(n->out_max);
    }
    if(*size == *max){
        /* We have to reallocate memory */
        *max = *max * 2;
        *arr_p = realloc(*arr_p, *max * sizeof(struct edge *));         
    }
    e->connected_nodes++;
    (*arr_p)[*size] = e;
    (*size)++;
}

struct edge* nodesrch(struct node *n, nodeid_t id, DIRECTION d){
    //struct edge *result = NULL;
    if(d == IN){
        for(int i = 0; i < n->in_size; i++){
            if(n->in[i]->from->id == id){
                return n->in[i];
            }
        }
    }
    else{
        for(int i = 0; i < n->out_size; i++){
            if(n->out[i]->to->id == id){
                return n->out[i];
            }
        }
    }
    return NULL; /* Nothing was found */
}

void nodedele(struct node *n, struct edge *e, DIRECTION d){
    struct edge ***arr_p;
    int *size;
    if(d == IN){
        arr_p = &(n->in);
        size = &(n->in_size);
    }
    else{
        arr_p = &(n->out);
        size = &(n->out_size);
    }
    /* if no edge exists return */
    if(*size == 0) return;
    
    /* if only 1 edge exists */
    if(*size == 1){
        e->connected_nodes--;
        *size = 0;
        return;
    }
    /* general case */
    for(int i = 0; i < *size; i++){
        if((*arr_p)[i] == e){
            /* That's the one we want to delete */
            /* We switch this one with the last edge
             * and reduce size */
            e->connected_nodes--;
            (*arr_p)[i] = (*arr_p)[*size - 1];
            (*size)--;
            break;
        }
    }
}

void nodeprnt(struct node *n){
    printf("Vertex(%d) = ", n->id);
    for(int i = 0; i < n->out_size; i++){
        if(i == n->out_size - 1){
            printf("(%d,%f)\n", n->out[i]->to->id, n->out[i]->amount);
        }
        else{
            printf("(%d,%f),", n->out[i]->to->id, n->out[i]->amount);
        }
    }
    if(n->out_size == 0){
        printf("\n");
    }
}

struct edge * edgecrt(struct node *n1, struct node *n2, amount_t amount){
    struct edge *e = malloc(sizeof(struct edge));
    if(e == NULL){
        return NULL;
    }
    e->from = n1;
    e->to = n2;
    e->amount = amount;
    e->connected_nodes = 0;
    return e;
}

void edgeupd(struct edge *e, amount_t plusamount){
    e->amount = e->amount + plusamount;
}

void edgedel(struct edge *e){
    free(e);
}

void graphcleanflags(struct node **visited_arr, int size){
    for(int i = 0; i < size; i++){
        visited_arr[i]->visited_flag = FALSE;
        visited_arr[i]->used_flag = FALSE;
    }
}

void graphfindpath(struct node *start_node, struct node *end_node){
    nodeid_t target_id = end_node->id;
    struct node *v;
    int arr_size = 0;
    int max_size = 100;
    struct node **visited_arr = malloc(sizeof(struct node *) * max_size);
    struct stack *stack = stackcrt();
    struct stack *tmp = NULL;
    bool found = FALSE;
    v = start_node;
    while(v->id != target_id){
        found = FALSE;
        for(int i = 0; i < v->out_size; i++){
            if(v->out[i]->to->visited_flag == FALSE
               && v->out[i]->to->id != start_node->id){
                stackpush(stack, v);
                v->out[i]->to->visited_flag = TRUE;

                /* update visited array */
                if(arr_size == max_size){
                    max_size *= 2;
                    visited_arr = realloc(visited_arr, max_size);
                }
                visited_arr[arr_size++] = v->out[i]->to;

                /* follow edge */
                v = v->out[i]->to;
                found = TRUE;
                break;
            }
        }
        if(found) continue;
        if(stacksize(stack) == 0){
            printf("conn (%d,%d) not found \n", start_node->id, end_node->id);
            graphcleanflags(visited_arr, arr_size);

            /* Free stacks and dynamic memory */
            stackdel(stack);
            free(visited_arr);
            return ;
        }
        v = stackpop(stack);
    }
    stackpush(stack, end_node);
    
    /* Reverse nodes for printing */
    tmp = stackrev(stack);
    int s_max = stacksize(tmp);
    printf("conn (%d,%d) = (", start_node->id, end_node->id);
    for(int i = 0; i < s_max; i++){
        if(stacksize(tmp) == 1){
            printf("%d", stackpop(tmp)->id);
        }
        else{
            printf("%d,", stackpop(tmp)->id);
        }
    }
    printf(")\n");
    
    /* Clean up visited flags */
    graphcleanflags(visited_arr, arr_size);
    
    /* Free stacks and dynamic memory */
    stackdel(stack);
    stackdel(tmp);
    free(visited_arr);
}

void graphfindcycles(struct node *start_node){
    nodeid_t target_id = start_node->id;
    struct node *v;
    int arr_size = 0;
    int max_size = 100;
    struct node **visited_arr = malloc(sizeof(struct node *) * max_size);
    struct stack *stack = NULL;
    struct stack *tmp = NULL;
    bool found = FALSE;
    int cycles = 0;
    int i;
    v = start_node;
    stack = stackcrt();
    printf("cycles %d = \n", start_node->id);
    while(1){
        found = FALSE;
        for(i = 0; i < v->out_size; i++){
            if(v->out[i]->to->visited_flag == FALSE
               || (v->out[i]->to->id == target_id && !v->used_flag)){
                v->out[i]->to->visited_flag = TRUE;
                stackpush(stack, v);

                /* Update visited edges */
                if(arr_size == max_size){
                    max_size *= 2;
                    visited_arr = realloc(visited_arr, max_size);
                }
                visited_arr[arr_size++] = v->out[i]->to;               
                
                /* Follow edge */
                v = v->out[i]->to;
                found = TRUE;
                break;
            }
        }

        if(found){
            if(v->id == target_id){
                /* We take in account cycles with 3 or more nodes */
                if(stacksize(stack) >= 3){
                    cycles++;
                    
                    /* Reverse nodes for printing */
                    printf("(");
                    if(tmp != NULL){
                        free(tmp);
                        tmp = NULL;
                    }
                    tmp = stackrev(stack);
                    int s_max = stacksize(tmp);
                    for(int i = 0; i < s_max; i++){
                        if(stacksize(tmp) == 1){
                            printf("%d", stackpop(tmp)->id);
                        }
                        else{
                            printf("%d,", stackpop(tmp)->id);
                        }
                    }
                    printf(")\n");
                }
                v = stackpop(stack);
                v->used_flag = TRUE;
            }
            continue;
        }
        if(stacksize(stack) == 0 ){
            /* No more cycles */
            break;
        }
        v = stackpop(stack);
    }

    if(cycles == 0){
        printf("No cycle found for %d\n", start_node->id);
    }
    graphcleanflags(visited_arr, arr_size);

    /* Free stacks and dynamic memory */
    stackdel(stack);
    if(tmp != NULL) stackdel(tmp);
    free(visited_arr);
}


void graphfindflow(struct node *start_node, int l){
    //nodeid_t target_id = start_node->id;
    struct node *n1;
    struct node *v;
    int arr_size = 0;
    int max_size = 100;
    struct node **visited_arr = malloc(sizeof(struct node *) * max_size);
    struct stack *stack = NULL;
    struct stack *tmp = NULL;
    bool found = FALSE;
    int traces = 0;
    int i;
    int depth = -1;
    float amount = 0;
    //float last_amount = 0;
    v = start_node;
    stack = stackcrt();
    while(1){
        found = FALSE;
        for(i = 0; i < v->out_size; i++){
            //printf("Checking %d->%d(%d): ", v->id, v->out[i]->to->id, depth);
            if((v->out[i]->to->visited_flag == FALSE)
               && (v->out[i]->to->id != start_node->id)
                ){
                //printf("IN\n");
                v->out[i]->to->visited_flag = TRUE;
                stackpush(stack, v);
                depth++;
                //last_amount = amount;
                amount += v->out[i]->amount;
                /* Update visited edges */
                if(arr_size == max_size){
                    max_size *= 2;
                    visited_arr = realloc(visited_arr, max_size);
                }
                visited_arr[arr_size++] = v->out[i]->to;               
                
                /* Follow edge */
                v = v->out[i]->to;
                found = TRUE;
                break;
            }
            //printf("OUT\n");
        }

        if(found){
            if(depth == l){
                traces++;
                /* Reverse nodes for printing */
                printf("(");
                if(tmp != NULL){
                    free(tmp);
                    tmp = NULL;
                }
                tmp = stackrev(stack);
                int s_max = stacksize(tmp);
                for(int i = 0; i < s_max; i++){
                    printf("%d,", stackpop(tmp)->id);
                }
                printf("%f)\n", amount);
                n1 = v;
                v = stackpop(stack);
                for(int j = 0; j < v->out_size; j++){
                    if(v->out[j]->to->id == n1->id){
                        amount -= v->out[j]->amount;
                        break;
                    }
                }
                depth--;
                v->used_flag = TRUE;
            }
            continue;
        }
        if(stacksize(stack) == 0 ){
            /* No more cycles */
            break;
        }
        n1 = v;
        v = stackpop(stack);
        for(int j = 0; j < v->out_size; j++){
            if(v->out[j]->to->id == n1->id){
                amount -= v->out[j]->amount;
                break;
            }
        }
        depth--;
    }

    if(traces++ == 0){
        printf("No flow found for %d and depth %d\n", start_node->id, l);
    }
    graphcleanflags(visited_arr, arr_size);

    /* Free stacks and dynamic memory */
    stackdel(stack);
    if(tmp != NULL) stackdel(tmp);
    free(visited_arr);
}

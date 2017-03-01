#include "stack.h"
#include <stdlib.h>

struct stack * stackcrt(){
    struct stack* mystack = malloc(sizeof(struct stack));
    mystack->top = NULL;
    mystack->size = 0;
    return mystack;
}

struct stack * stackrev(struct stack *s){
    struct stack* newstack = stackcrt();
    struct stack_node *n;
    n = s->top;
    while(n != NULL){
        stackpush(newstack, n->item);
        n = n->next;
    }
    return newstack;
}

void stackpush(struct stack *s, data_type item){
    struct stack_node *sn = malloc(sizeof(struct stack_node));
    sn->item = item;
    sn->next = s->top;
    s->top = sn;
    s->size++;
}

data_type stackpop(struct stack *s){
    if(s->size == 0) return NULL;
    data_type item = s->top->item;
    struct stack_node *temp = s->top;
    s->top = s->top->next;
    s->size--;
    free(temp);
    return item;
}

data_type stackfetch(struct stack *s){
    return s->top->item;
}

int stacksize(struct stack *s){
    return s->size;
}

void stackdel(struct stack *s){
    struct stack_node *current = s->top;
    struct stack_node *temp;
    while(current != NULL){
        temp = current;
        current = current->next;
        free(temp);
    }
    free(s);
}

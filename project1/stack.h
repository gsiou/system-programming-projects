#define data_type struct node*

/* Data structures */

struct stack_node{
    data_type item;
    struct stack_node *next;
};

struct stack{
    int size;
    struct stack_node *top;
};

/* Functions */

/* Creates and returns a stack */
struct stack * stackcrt();

/* Constructs a stack identical to one given, except reverse */
struct stack * stackrev(struct stack *s);

/* Adds an item to the top of the stack */
void stackpush(struct stack * s, data_type item);

/* Removes and returns stack's top item */
data_type stackpop(struct stack *s);

/* Returns stack's top item without removing it */
data_type stackfetch(struct stack *s);

/* Returns number of items in stuck */
int stacksize(struct stack *s);

/* Deletes stack and frees memory */
void stackdel(struct stack *s);

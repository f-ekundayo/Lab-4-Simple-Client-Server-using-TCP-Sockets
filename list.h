#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct node {
  int value;
  struct node *next;
} node_t;

typedef struct {
  node_t *head;
  size_t size;
} list_t;

/* Allocation / teardown */
list_t* list_alloc(void);
void     list_free(list_t *list);

/* Mutators */
void list_add_to_front(list_t *list, int value);
void list_add_to_back(list_t *list, int value);
int  list_add_at_index(list_t *list, int index, int value); /* 0 on ok, -1 bad index */

/* Removers (return removed value; if empty/bad index return INT_MIN) */
int  list_remove_from_front(list_t *list);
int  list_remove_from_back(list_t *list);
int  list_remove_at_index(list_t *list, int index);

/* Accessors */
int   list_get_elem_at(list_t *list, int index); /* returns INT_MIN if OOB */
size_t list_length(list_t *list);

/* String view: returns pointer to internal static buffer */
char* listToString(list_t *list);

#endif /* LIST_H */

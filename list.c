#include "list.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

list_t* list_alloc(void) {
  list_t *l = (list_t*)malloc(sizeof(list_t));
  if (!l) return NULL;
  l->head = NULL;
  l->size = 0;
  return l;
}

void list_free(list_t *list) {
  if (!list) return;
  node_t *cur = list->head;
  while (cur) {
    node_t *nxt = cur->next;
    free(cur);
    cur = nxt;
  }
  free(list);
}

void list_add_to_front(list_t *list, int value) {
  node_t *n = (node_t*)malloc(sizeof(node_t));
  n->value = value;
  n->next = list->head;
  list->head = n;
  list->size++;
}

void list_add_to_back(list_t *list, int value) {
  node_t *n = (node_t*)malloc(sizeof(node_t));
  n->value = value;
  n->next = NULL;
  if (!list->head) {
    list->head = n;
  } else {
    node_t *cur = list->head;
    while (cur->next) cur = cur->next;
    cur->next = n;
  }
  list->size++;
}

int list_add_at_index(list_t *list, int index, int value) {
  if (index < 0 || (size_t)index > list->size) return -1;
  if (index == 0) { list_add_to_front(list, value); return 0; }
  node_t *n = (node_t*)malloc(sizeof(node_t));
  n->value = value;
  node_t *cur = list->head;
  for (int i = 0; i < index-1; i++) cur = cur->next;
  n->next = cur->next;
  cur->next = n;
  list->size++;
  return 0;
}

int list_remove_from_front(list_t *list) {
  if (!list->head) return INT_MIN;
  node_t *old = list->head;
  int v = old->value;
  list->head = old->next;
  free(old);
  list->size--;
  return v;
}

int list_remove_from_back(list_t *list) {
  if (!list->head) return INT_MIN;
  if (!list->head->next) {
    int v = list->head->value;
    free(list->head);
    list->head = NULL;
    list->size--;
    return v;
  }
  node_t *cur = list->head;
  while (cur->next && cur->next->next) cur = cur->next;
  int v = cur->next->value;
  free(cur->next);
  cur->next = NULL;
  list->size--;
  return v;
}

int list_remove_at_index(list_t *list, int index) {
  if (index < 0 || (size_t)index >= list->size) return INT_MIN;
  if (index == 0) return list_remove_from_front(list);
  node_t *cur = list->head;
  for (int i = 0; i < index-1; i++) cur = cur->next;
  node_t *t = cur->next;
  int v = t->value;
  cur->next = t->next;
  free(t);
  list->size--;
  return v;
}

int list_get_elem_at(list_t *list, int index) {
  if (index < 0 || (size_t)index >= list->size) return INT_MIN;
  node_t *cur = list->head;
  for (int i = 0; i < index; i++) cur = cur->next;
  return cur->value;
}

size_t list_length(list_t *list) {
  return list->size;
}

char* listToString(list_t *list) {
  /* Static buffer is fine for simple labs; adjust if you need more */
  static char buf[4096];
  char tmp[64];
  buf[0] = '\0';
  strcat(buf, "[");
  node_t *cur = list->head;
  while (cur) {
    snprintf(tmp, sizeof(tmp), "%d", cur->value);
    strcat(buf, tmp);
    if (cur->next) strcat(buf, ", ");
    cur = cur->next;
  }
  strcat(buf, "]");
  return buf;
}

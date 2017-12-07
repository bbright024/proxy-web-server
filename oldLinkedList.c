#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "LinkedList.h"
#include "LinkedList_priv.h"

#ifndef _LINKEDLIST_PRIV_H_
#define _LINKEDLIST_PRIV_H_

#include <stdint.h>      // for uint64_t
#include "./LinkedList.h"  // for LinkedList and LLIter

typedef struct ll_node {
  void      *payload;		/* customer-supplied payload pointer */
  struct ll_node *next;		/* next node in list, or NULL */
  struct ll_node *prev;		/* prev node in list, or NULL */
} LinkedListNode, *LinkedListNodePtr;

typedef struct ll_head {
  uint64_t    num_elements; 	
  LinkedListNodePtr head;	/* head of list, NULL if empty */
  LinkedListNodePtr tail;	/* tail of list, NULL if empty */
} LinkedListHead;

typedef struct ll_iter {
  LinkedList    ist;		/* the list we're for */
  LinkedListNodePtr node;	/* the node we're at, NULL if broken */
} LLIterSt;

#endif // _LINKEDLIST_PRIV_H_

LinkedList AllocateLinkedList(void)
{
  LinkedList li;
  li = malloc(sizeof(LinkedListHead));
  if (li == 0)
    return NULL;
  li->num_elements = 0;
  li->head = NULL;
  li->tail = NULL;
  return li;
}

void FreeLinkedList(LinkedList list,
		    LLPayloadFreeFnPtr payload_free_function)
{
  assert(list);
  assert(payload_free_function);
    
  LinkedListNodePtr lip_cur, lip_next;
  lip_next = list->head;
  while (lip_next) {
    lip_cur = lip_next;
    lip_next = lip_next->next;
    payload_free_function(lip_cur->payload);
    free(lip_cur);
  }
  free(list);
}

uint64_t NumElementsInLinkedList(LinkedList list)
{
  assert(list);
  return list->num_elements;
}

bool PushLinkedList(LinkedList list, void *payload)
{
  assert(list);
  LInkedListNodePtr lip_new;

  lip_new = malloc(sizeof(struct ll_node));
  if (!lip_new)
    return false;
  lip_new->payload = payload;
  lip_new->next = list->head;
  if (list->head) {
    list->head->prev = lip_new;
  }
  else {
    list->head = lip_new;
    list->prev = lip_new;
  }
  lip_new->prev = NULL;
  list->head = lip_new;
  return true;
}

bool PopLinkedList(LinkedList list, void **payload_ptr)
{
  assert(list && payload_ptr);
  if (!list->head)
    return false;
  *payload_ptr = list->head->payload;

  LinkedListNodePtr temp;
  temp = list->head;
  list->head = list->head->next;
  free(temp);
  list->num_elements--;
  
  
  if (list->head){
    list->head->prev = NULL;
    list->tail = NULL;
  }
  else {
    
  }

  
  
  
}

bool AppendLinkedList(LinkedList list, void *payload);

bool SliceLinkedList(LinkedList list, void **payload_ptr);

void SortLinkedList(LinkedList list, unsigned int ascending,
                    LLPayloadComparatorFnPtr comparator_function);


/* now ll_iter functions */

//struct ll_iter;
//typedef struct ll_iter *LLIter;  // same trick to hide implementation.

LLIter LLMakeIterator(LinkedList list, int pos);

void LLIteratorFree(LLIter iter);

bool LLIteratorHasNext(LLIter iter);

bool LLIteratorNext(LLIter iter);

bool LLIteratorHasPrev(LLIter iter);

bool LLIteratorPrev(LLIter iter);

void LLIteratorGetPayload(LLIter iter, void **payload);

bool LLIteratorDelete(LLIter iter,
                      LLPayloadFreeFnPtr payload_free_function);

bool LLIteratorInsertBefore(LLIter iter, void *payload);

/* 
 * Brian Bright 5/1/2017
 * 
 * This file is my implementation of UW CSE 333 assignment 1.
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <includes/LinkedList.h>
#include <includes/LinkedList_priv.h>
//#include "includes/LinkedList.h"
//#include "includes/LinkedList_priv.h"

/* 
void PrintLinkedList(LinkedList list)
{
  assert(list);
  LLIter iter = LLMakeIterator(list, 0);
  int *payload;

  printf("[");
  
  if (iter->node) {

    LLIteratorGetPayload(iter, (void *)&payload);
    printf("p: %d]-->[", *(int *)payload);
    
    while (LLIteratorHasNext(iter)) {
      LLIteratorNext(iter);
      LLIteratorGetPayload(iter, (void *)&payload);
      printf("p: %d]-->[", *(int *)payload);
    } 
  }

  printf("NULL]\n");
  LLIteratorFree(iter);
}
*/

LinkedList AllocateLinkedList(void)
{
  LinkedList the_list = malloc(sizeof(LinkedListHead));

	if(the_list == 0)
		return NULL;
	the_list->num_elements = 0;
	the_list->head = NULL;
	the_list->tail = NULL;
	return the_list;
}

void FreeLinkedList(LinkedList list,
                    LLPayloadFreeFnPtr payload_free_function)
{
  assert(list);
  assert(payload_free_function);
  
  LinkedListNodePtr curr_node = list->head;
  LinkedListNodePtr temp;
  
  uint64_t i;
  for (i = 0; i < list->num_elements; i++) {
    temp = curr_node->next;
    payload_free_function(curr_node->payload);
    free(curr_node);
    curr_node = temp;
  }
  free(list);
}

uint64_t NumElementsInLinkedList(LinkedList list)
{
	assert(list != NULL);
	return list->num_elements;
}

bool PushLinkedList(LinkedList list, void *payload)
{
	assert(list);
	LinkedListNodePtr new_node = malloc(sizeof(LinkedListNode));

	if(new_node == NULL)
		return false;

	new_node->payload = payload;
	new_node->next = list->head;
	new_node->prev = NULL;
	if (list->head) {
	  list->head->prev = new_node;
	}
	else {
	  list->head = new_node;
	  list->tail = new_node;
	}
	
	list->head = new_node;
	list->num_elements += 1;
	return true;
}

/* remove from beginning */
bool PopLinkedList(LinkedList list, void **payload_ptr)
{
	assert(payload_ptr && list);

	if (list->num_elements == 0)
		return false;
	*payload_ptr = list->head->payload;

	LinkedListNodePtr temp = list->head;

	if (list->num_elements == 1) {
		// the case that the list has only one node
		list->head = NULL;
		list->tail = NULL;
	} else {
		// the case that list has at least two nodes
		list->head = list->head->next;
		list->head->prev = NULL;
	}

	list->num_elements -= 1;
	free(temp);
	return true;
}

bool AppendLinkedList(LinkedList list, void *payload)
{
	assert(list != NULL);
	LinkedListNodePtr end_node = list->tail;
	LinkedListNodePtr new_node = malloc(sizeof(struct ll_node));

	if(new_node == NULL)
		return false;

	new_node->payload = payload;
	new_node->next = NULL;

	if(list->num_elements == 0)				/* case 1: empty list, payload is first node */
		{
			assert(list->head == NULL);
			assert(list->tail == NULL);
			
			list->num_elements = 1;
			list->tail = list->head = new_node;
			
			new_node->prev = NULL;
			return true;

		}

	/* stitch the nodes */
	new_node->prev = end_node;
	end_node->next = new_node;
	list->num_elements += 1;
	return true;
}

bool SliceLinkedList(LinkedList list, void **payload_ptr)
{
	assert(list != NULL);
	assert(payload_ptr != NULL);

	if(list->num_elements == 0) /* case 1: empty list */
		return false;

	assert(list->tail != NULL);
	assert(list->head != NULL);
	
	if(list->num_elements == 1)	/* case 2: single element list */
		{
			*payload_ptr = list->head->payload;
			free(list->head);
			list->head = list->tail = NULL;
			list->num_elements = 0;
			return true;
		}

	/* case 3: multi element list */
	LinkedListNodePtr end_node = list->tail;
	*payload_ptr = end_node->payload;
	list->tail = end_node->prev;
	list->tail->next = NULL;
	free(end_node);
	list->num_elements -= 1;
	return true;
}

void SortLinkedList(LinkedList list, unsigned int ascending,
                    LLPayloadComparatorFnPtr comparator_function)
{
	assert(list != NULL);
	assert(list->head != NULL);
	assert(list->tail != NULL);

	if(list->num_elements < 2)
		return;

	int swapped, comp;
	LinkedListNodePtr curr_node;
	void *temp_payload;
	do {
		swapped = 0;
		curr_node = list->head;
		while(curr_node->next != NULL)
			{
				comp = comparator_function(curr_node->payload, curr_node->next->payload);

				if(ascending)
					comp *= -1;

				if(comp < 0)
					{
						temp_payload = curr_node->payload;
						curr_node->payload = curr_node->next->payload;
						curr_node->next->payload = temp_payload;
						swapped = 1;
					}
				curr_node = curr_node->next;
			}
	} while(swapped);
}

LLIter LLMakeIterator(LinkedList list, int pos)
{
	assert(list != NULL);
	assert(pos == 0 || pos == 1);

	LLIter the_iter;
	
	if( (the_iter = malloc(sizeof(struct ll_iter))) == NULL)
		return (LLIter)NULL;
	
	the_iter->list = list;
	the_iter->node = (pos) ? list->tail : list->head;
	return the_iter;
}

void LLIteratorFree(LLIter iter)
{
	assert(iter != NULL);
	free(iter);
}

bool LLIteratorHasNext(LLIter iter)
{
	assert(iter != NULL);
	assert(iter->node != NULL);
	assert(iter->list != NULL);
	
	if(iter->node->next != NULL)
		return true;

	return false;
}

bool LLIteratorNext(LLIter iter)
{
	assert(iter != NULL);
	assert(iter->list != NULL);
	assert(iter->node != NULL);

	if(iter->node->next != NULL)
		{
			iter->node = iter->node->next;
			return true;
		}

	return false;
}

bool LLIteratorHasPrev(LLIter iter)
{
	assert(iter != NULL);
	assert(iter->list != NULL);
	assert(iter->node != NULL);

	if(iter->node->prev != NULL)
		return true;

	return false;
}


bool LLIteratorPrev(LLIter iter)
{
	assert(iter != NULL);
	assert(iter->list != NULL);
	assert(iter->node != NULL);

	if(iter->node->prev != NULL)
		{
			iter->node = iter->node->prev;
			return true;
		}
	return false;
}

void LLIteratorGetPayload(LLIter iter, void **payload)
{
	assert(iter != NULL);
	assert(iter->list != NULL);
	assert(iter->node != NULL);

	*payload = iter->node->payload;
}

bool LLIteratorDelete(LLIter iter,
                      LLPayloadFreeFnPtr payload_free_function)
{
	assert(iter != NULL);
	assert(iter->list != NULL);
	assert(iter->node != NULL);
	LinkedListNodePtr temp_node;
	
	if(iter->list->num_elements == 1) /* case 1: 1 node in list */
		{
			payload_free_function(iter->node);
			iter->list->num_elements = 0;
			return false;
		}

	if(iter->node->next == NULL) /* case 2: iter is at the tail */
		{
			temp_node = iter->node->prev;
			temp_node->next = NULL;
			
			payload_free_function(iter->node->payload);
			free(iter->node);
			iter->node = temp_node;
			
			iter->list->num_elements -= 1;
			return true;
		}
	if(iter->node->prev == NULL) /* case 3: iter is at head */
		{
			temp_node = iter->node->next;
			temp_node->prev = NULL;
			
			payload_free_function(iter->node->payload);
			free(iter->node);
			iter->node = temp_node;
			iter->list->head = temp_node;
			iter->list->num_elements -= 1;
			return true;
		}
	/* case 4: iter in a list of multiple nodes & not at the end */
	temp_node = iter->node->next;
	temp_node->prev = iter->node->prev;
	iter->node->prev->next = temp_node;

	payload_free_function(iter->node->payload);
	free(iter->node);
	iter->node = temp_node;
	
	iter->list->num_elements -= 1;
	return true;
}
/* implement LRU algorithm */
bool LLIteratorMoveToHead(LLIter iter)
{
  assert(iter && iter->list && iter->node);
  if (iter->list->num_elements < 2)
    return true;
  if (iter->list->head == iter->node)
    return true;
  
  LinkedListNodePtr temp_head, temp_prev, temp_next;
  temp_head = iter->list->head;
  temp_prev = iter->node->prev;
  temp_next = iter->node->next;

  /* update new head */
  temp_head->prev = iter->node;
  iter->node->next = temp_head;
  iter->node->prev = NULL;
  iter->list->head = iter->node;
  
  temp_prev->next = temp_next;
  if (temp_next)
    temp_next->prev = temp_prev;
  else
    iter->list->tail = temp_prev;

  return true;

}
bool LLIteratorInsertBefore(LLIter iter, void *payload)
{
	assert(iter != NULL);
	assert(iter->list != NULL);
	assert(iter->node != NULL);

	if(iter->node->prev == NULL) /* case 1: iter is at head */
		{
			return PushLinkedList(iter->list, payload);
		}

	/* case 2: iter passed the head */
	LinkedListNodePtr new_node, temp_prev;
	if( (new_node = malloc(sizeof(struct ll_node))) == NULL)
		return false;

	new_node->payload = payload;
	
	/* stitch the nodes together */
	temp_prev = iter->node->prev;
	temp_prev->next = new_node;
		
	new_node->prev = temp_prev;
	new_node->next = iter->node;
	
	iter->node->prev = new_node;
	iter->list->num_elements += 1;
	
	return true;
}



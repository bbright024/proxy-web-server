#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <includes/LinkedList.h>

#define N 100
// the payload type we'll add
typedef struct {
  int num;
} ExamplePayload, *ExamplePayloadPtr;

// prototype of our payload free function
void ExamplePayloadFree(void *payload);

// prototype of our payload comparator function
int ExamplePayloadComparator(void *p1, void *p2);

void print_list(LinkedList list);

// our main function; here, we demonstrate how to use some
// of the linked list functions.
int main(int argc, char **argv) {
  ExamplePayloadPtr payload;
  LinkedList list;
  LLIter iter;
  int i;


  // allocate a list
  list = AllocateLinkedList();
  assert(list != NULL);

  // insert 100 elements
  for (i = 0; i < 100; i++) {
    payload = (ExamplePayloadPtr) malloc(sizeof(ExamplePayload));
    assert(payload != NULL);
    payload->num = i;
    assert(PushLinkedList(list, (void *) payload) == 1);

    // make sure our list total is correct
    assert(NumElementsInLinkedList(list) == i+1);
  }
  
  // sort the list in descending order
  SortLinkedList(list, 0, &ExamplePayloadComparator);

  //  print_list(list);

  // pop off the first element
  assert(PopLinkedList(list, (void **) &payload) == 1);
  assert(payload->num == 99);
  assert(NumElementsInLinkedList(list) == 99);
  free(payload);

  
  // slice off the last element
  assert(SliceLinkedList(list, (void **) &payload) == 1);
  assert(payload->num == 0);
  assert(NumElementsInLinkedList(list) == 98);
  free(payload);

  // make an iterator from the head
  iter = LLMakeIterator(list, 0);
  assert(iter != NULL);

  // peek at the current iterator payload
  LLIteratorGetPayload(iter, (void **) &payload);
  assert(payload->num == 98);

  // move the iterator, peek at next payload
  assert(LLIteratorNext(iter) == 1);
  LLIteratorGetPayload(iter, (void **) &payload);
  assert(payload->num == 97);

  for (i = 0; i < N; i++) {
    payload = (ExamplePayloadPtr) malloc(sizeof(ExamplePayload));
    assert(payload != NULL);
    payload->num = i;
    assert(PushLinkedList(list, (void *) payload) == 1);
   
  }
  LLIteratorFree(iter);
  iter = LLMakeIterator(list, 0);
  assert(iter != NULL);
  
  for (i = 0; i < N; i++)
	  {
		  if(!LLIteratorDelete(iter, &ExamplePayloadFree))
			  {
				  /* list is now empty */
				  break;
			  }
	  }
  
  // free the iterator
  LLIteratorFree(iter);
  //  print_list(list);
  // free the linked list
  FreeLinkedList(list, &ExamplePayloadFree);
  return 0;
}


void ExamplePayloadFree(void *payload) {
  assert(payload != NULL);
  free(payload);
}

// prototype of our payload comparator function
int ExamplePayloadComparator(void *p1, void *p2) {
  int i1, i2;
  assert(p1 != NULL);
  assert(p2 != NULL);

  i1 = ((ExamplePayloadPtr) p1)->num;
  i2 = ((ExamplePayloadPtr) p2)->num;

  if (i1 > i2)
    return 1;
  if (i1 < i2)
    return -1;
  return 0;
}


void print_list(LinkedList list)
{
	LLIter iter = LLMakeIterator(list, 0);
	ExamplePayloadPtr payload = malloc(sizeof(ExamplePayload));
	int i = 0;
	while(LLIteratorHasNext(iter))
		{
			LLIteratorGetPayload(iter, (void **) &payload);
			
			printf("Node %d = %d   ", i, payload->num);
			LLIteratorNext(iter);
			i++;
		}
	ExamplePayloadFree(payload);
	LLIteratorFree(iter);
	
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <includes/LinkedList.h>

#define N 10

typedef struct {
  int num;
} Test;

void TestPayloadFree(void *payload);
int TestPayloadComparator(void *p1, void *p2);

//void print_list(LinkedList list);

// our main function; here, we demonstrate how to use some
// of the linked list functions.
int main(int argc, char **argv) {
  Test *payload;
  LinkedList list;
  LLIter iter;
  int i;

  list = AllocateLinkedList();
  assert(list != NULL);

  // insert N elements
  for (i = 0; i < N; i++) {
    payload = (Test *) malloc(sizeof(Test));
    assert(payload != NULL);
    payload->num = i;
    assert(PushLinkedList(list, (void *) payload) == 1);

    // make sure our list total is correct
    assert(NumElementsInLinkedList(list) == i+1);
  }
  // sort the list in descending order
  SortLinkedList(list, 0, &TestPayloadComparator);

  // pop off the first element
  assert(PopLinkedList(list, (void **) &payload) == 1);
  assert(payload->num == (N - 1));
  assert(NumElementsInLinkedList(list) == (N - 1));
  free(payload);

  
  // slice off the last element
  assert(SliceLinkedList(list, (void **) &payload) == 1);
  assert(payload->num == 0);
  assert(NumElementsInLinkedList(list) == (N - 2));
  free(payload);

  // make an iterator from the head
  iter = LLMakeIterator(list, 0);
  assert(iter != NULL);

  // peek at the current iterator payload
  LLIteratorGetPayload(iter, (void **) &payload);
  assert(payload->num == (N - 2));

  // move the iterator, peek at next payload
  assert(LLIteratorNext(iter) == 1);
  LLIteratorGetPayload(iter, (void **) &payload);
  assert(payload->num == (N - 3));

  for (i = 0; i < N; i++) {
    payload = (Test *) malloc(sizeof(Test));
    assert(payload != NULL);
    payload->num = i;
    assert(PushLinkedList(list, (void *) payload) == 1);
  }
  
  LLIteratorFree(iter);
  iter = LLMakeIterator(list, 0);
  assert(iter != NULL);
  
  for (i = 0; i < N; i++) {
    if(!LLIteratorDelete(iter, &TestPayloadFree)) {
      /* list is now empty */
      break;
    }
  }
  //  SortLinkedList(list, 0, &TestPayloadComparator);
  // free the iterator
  LLIteratorFree(iter);
  //  print_list(list);
  // free the linked list
  FreeLinkedList(list, &TestPayloadFree);
  return 0;
}


void TestPayloadFree(void *payload) {
  assert(payload != NULL);
  free(payload);
}

// prototype of our payload comparator function
int TestPayloadComparator(void *p1, void *p2) {
  int i1, i2;
  assert(p1 != NULL);
  assert(p2 != NULL);

  i1 = ((Test *) p1)->num;
  i2 = ((Test *) p2)->num;

  if (i1 > i2)
    return 1;
  if (i1 < i2)
    return -1;
  return 0;
}

/* 
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
 */

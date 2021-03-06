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

  if (LLIteratorHasPrev(iter))
    assert(0);
  if (LLIteratorPrev(iter))
    assert(0);
  
  // peek at the current iterator payload
  LLIteratorGetPayload(iter, (void **) &payload);
  assert(payload->num == (N - 2));

  // move the iterator, peek at next payload
  assert(LLIteratorNext(iter) == 1);
  if (LLIteratorHasPrev(iter)) {
    LLIteratorPrev(iter);
    LLIteratorNext(iter);
  }
  else
    assert(0);
  
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
  LLIteratorMoveToHead(iter);

  payload = (Test *) malloc(sizeof(Test));
  payload->num = i;
  LLIteratorInsertBefore(iter, (void *) payload);
  LLIteratorNext(iter);
  while(LLIteratorDelete(iter, &TestPayloadFree))
    ;
  LLIteratorFree(iter);
  FreeLinkedList(list, &TestPayloadFree);

  /* test corner cases */
  Test *payload2;
  Test *payload3;
  Test *payload4;
  
  list = AllocateLinkedList();
  payload = (Test *) malloc(sizeof(Test));
  assert(payload != NULL);
  payload->num = 1;

  payload2 = (Test *) malloc(sizeof(Test));
  assert(payload2 != NULL);
  payload2->num = 2;

  payload3 = (Test *) malloc(sizeof(Test));
  assert(payload3 != NULL);
  payload3->num = 3;

  payload4 = (Test *) malloc(sizeof(Test));
  assert(payload4 != NULL);
  payload4->num = 4;
  
  assert(AppendLinkedList(list, (void *) payload) == 1);

  PrintLinkedList(list);
  SortLinkedList(list, 0, &TestPayloadComparator);
  SortLinkedList(list, 1, &TestPayloadComparator);
  
  while(PopLinkedList(list, (void **) &payload))
    ;

  //  assert(AppendLinkedList(list, (void *) payload) == 1);  
  assert(AppendLinkedList(list, (void *) payload2) == 1);
  assert(AppendLinkedList(list, (void *) payload3) == 1);
  
  

  iter = LLMakeIterator(list, 1);
  LLIteratorPrev(iter);
  assert(LLIteratorInsertBefore(iter, (void *)payload4) == 1);
  LLIteratorMoveToHead(iter);
  LLIteratorMoveToHead(iter);
  LLIteratorFree(iter);
  //SliceLinkedList(list, (void *) &payload);
  //  SliceLinkedList(list, (void **) &payload);
  //SliceLinkedList(list, (void **) &payload);

  //assert(AppendLinkedList(list, (void *) payload) == 1);  
  //  assert(AppendLinkedList(list, (void *) payload2) == 1);
  PrintLinkedList(list);
  SortLinkedList(list, 0, &TestPayloadComparator);
  SortLinkedList(list, 1, &TestPayloadComparator);

  while(PopLinkedList(list, (void **) &payload))
      ;

  //  if (SliceLinkedList(list, (void **) &payload))
  //    assert(0);
  FreeLinkedList(list, &TestPayloadFree);


  LinkedList slice_test = AllocateLinkedList();
  int *pyld = malloc(sizeof(int));
  SliceLinkedList(slice_test, &pyld);
  
  int *rand0 = malloc(sizeof(int));
  *rand0 = 1;

  PushLinkedList(slice_test, rand0);
  SliceLinkedList(slice_test, &pyld);
  FreeLinkedList(slice_test, free);

  
  LinkedList lru_test = AllocateLinkedList();

  int *rand1 = malloc(sizeof(int));
  int *rand2 = malloc(sizeof(int));
  int *rand3 = malloc(sizeof(int));
  *rand1 = 1;
  *rand2 = 2;
  *rand3 = 3;

  LLIter iter_lru = LLMakeIterator(lru_test, 1);
    
  PushLinkedList(lru_test, (void *)rand1);

  LLIteratorMoveToHead(iter_lru);
  LLIteratorFree(iter_lru);
  
  PushLinkedList(lru_test, (void *)rand2);
  PushLinkedList(lru_test, (void *)rand3);

  iter_lru = LLMakeIterator(lru_test, 1);
  PrintLinkedList(lru_test);

  LLIteratorMoveToHead(iter_lru);
  PrintLinkedList(lru_test);
  
  LLIteratorFree(iter_lru);
  FreeLinkedList(lru_test, free);


  LinkedList insert_b4 = AllocateLinkedList();
  rand1 = malloc(sizeof(int));
  rand2 = malloc(sizeof(int));
  rand3 = malloc(sizeof(int));
  *rand1 = 1;
  *rand2 = 2;
  *rand3 = 3;

  printf("\nTesting insert:\n");
  PrintLinkedList(insert_b4);
  
  LLIter iter_ins1 = LLMakeIterator(insert_b4, 0);
  LLIteratorInsertBefore(iter_ins1, (void *)rand2);
  printf("LLIteratorIB into empty list:\n");
  PrintLinkedList(insert_b4);
  //  PushLinkedList(insert_b4, (void *)rand2);
  LLIteratorFree(iter_ins1);
  
  PushLinkedList(insert_b4, (void *)rand3);
  printf("PushLL after LLIteratorIB into empty list:\n");
  PrintLinkedList(insert_b4);
  LLIter iter_ins = LLMakeIterator(insert_b4, 1);

  
  LLIteratorInsertBefore(iter_ins, (void *)rand1);
  printf("LLIteratorIB :\n");
  PrintLinkedList(insert_b4);
  
  LLIteratorFree(iter_ins);
  
  FreeLinkedList(insert_b4, free);
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

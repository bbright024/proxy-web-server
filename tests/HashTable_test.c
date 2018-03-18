#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <includes/HashTable.h>

#define ELEMENTS 25000
#define BUCKETS  100
typedef struct {
  int num;
} Test;

// prototype of our free function
void TestValueFree(void *value);

int main(int argc, char **argv) {
  Test *test_struct;
  HashTable ht;
  HTIter iter;
  HTKeyValue kv, old_kv;
  int i;

  ht = AllocateHashTable(0);
  assert(ht == NULL);
  ht = AllocateHashTable(BUCKETS);
  assert(ht != NULL);

  assert(NumElementsInHashTable(ht) == 0);
  
  // insert ELEMENTS (load factor = 2.0)
  for (i = 0; i < ELEMENTS; i++) {
    test_struct = (Test *) malloc(sizeof(Test));
    assert(test_struct != NULL);
    test_struct->num = i;

    // make sure HT has the right # of elements in it to start
    assert(NumElementsInHashTable(ht) == (uint64_t) i);

    kv.key = FNVHashInt64(i);
    kv.value = (void *) test_struct;
    assert(InsertHashTable(ht, kv, &old_kv) == 1);

	// make sure hash table has right # of elements post-insert
    assert(NumElementsInHashTable(ht) == (uint64_t) (i+1));
  }
  

  // look up a few values
  uint64_t val1 = ELEMENTS % 100;
  assert(LookupHashTable(ht, FNVHashInt64(val1), &kv) == 1);
  assert(kv.key == FNVHashInt64(val1));
  assert(((Test *) kv.value)->num == val1);

  uint64_t val2 = ELEMENTS - 100;
  assert(LookupHashTable(ht, FNVHashInt64(val2), &kv) == 1);
  assert(kv.key == FNVHashInt64(val2));
  assert(((Test *) kv.value)->num == val2);

  // make sure non-existent value cannot be found
  assert(LookupHashTable(ht, FNVHashInt64(ELEMENTS), &kv) == 0);

  // delete a value
  assert(RemoveFromHashTable(ht, FNVHashInt64(val1), &kv) == 1);
  assert(kv.key == FNVHashInt64(val1));
  assert(((Test *) kv.value)->num == val1);
  TestValueFree(kv.value);   // since we malloc'ed it, we must free it

  // make sure it's deleted
  assert(LookupHashTable(ht, FNVHashInt64(val1), &kv) == 0);
  assert(NumElementsInHashTable(ht) == (uint64_t) (ELEMENTS - 1));

  // loop through using an iterator
  i = 0;
  iter = HashTableMakeIterator(ht);
  assert(iter != NULL);
  //  printf("htiter starting\n");

  while (HTIteratorPastEnd(iter) == 0) {
    assert(HTIteratorGet(iter, &kv) == 1);
    assert(kv.key != FNVHashInt64(val1));   // we deleted it

    HTIteratorNext(iter);
    i++;
  }
  assert(i == (ELEMENTS - 1));


  //  printf("htiter advanced\n");
  HTIteratorFree(iter);
  //  printf("htiter freed\n");
  PrintHashTable(ht);
  FreeHashTable(ht, &TestValueFree);

  //  printf("ht freed\n");
  return 0;
}

void TestValueFree(void *value) {
  assert(value != NULL);
  free(value);
}

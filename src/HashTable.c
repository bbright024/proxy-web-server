/* 
 * Copyright 1969 bbb
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <assert.h>
#include <includes/HashTable.h>
#include <includes/HashTable_priv.h>

// struct htrec;
// typedef struct htrec *HashTable;
// This is the struct that we use to represent a hash table. Quite simply, a
// hash table is just an array of buckets, where each bucket is a linked list
// of HTKeyValue structs.

/* 
// This is the struct we use to represent an iterator.


// This is the internal hash function we use to map from uint64_t keys to a
// bucket number.
uint64_t HashKeyToBucketNum(HashTable ht, uint64_t key);
 */

// typedef void(*ValueFreeFnPtr)(void *value);

/* private utility function to grow the hashtable if load factor too high */
static void ResizeHashTable(HashTable ht);

/* free function that does nothing */
static void NullFree(void *freeme) { }


HashTable AllocateHashTable(uint32_t num_buckets)
{
  HashTable ht;
  uint32_t i;
  
  if (num_buckets == 0) {
    return NULL;
  }
  
  if ((ht = malloc(sizeof(HashTableRecord))) == NULL)
    return NULL;
  
  ht->num_buckets = num_buckets;
  ht->num_elements = 0;

  ht->buckets = (LinkedList *)malloc(num_buckets * sizeof(LinkedList));
  if(ht->buckets == NULL) {
    free(ht);
    return NULL;
  }
  
  for (i = 0; i < num_buckets; i++) {
    if ((ht->buckets[i] = AllocateLinkedList()) == NULL){
      uint32_t j;
      for (j = 0; j < i; j++)
	FreeLinkedList(ht->buckets[j], NullFree); /* cool trick, nothing in buckets yet*/
      free(ht);
      return NULL;
    }
    
  }

  return ht;
}

void FreeHashTable(HashTable table, ValueFreeFnPtr value_free_function)
{
  assert(table != NULL);
  uint64_t i, n;
  n = table->num_buckets;
  for (i = 0; i < n; i++) {
    FreeLinkedList(table->buckets[i], value_free_function);
  }
  free(table);
}

uint64_t NumElementsInHashTable(HashTable table)
{
	assert(table);
	return table->num_elements;
}
/* 
for reference!

typedef struct {
  uint64_t   key;    // the key in the key/value pair
  void      *value;  // the value in the key/value pair
} HTKeyValue, *HTKeyValuePtr;
 */
uint64_t FNVHash64(unsigned char *buffer, unsigned int len)
{
	
	static const uint64_t FNV1_64_INIT = 0xcbf29ce484222325ULL;
	static const uint64_t FNV_64_PRIME = 0x100000001b3ULL;
	unsigned char *bp = (unsigned char *) buffer;
	unsigned char *be = bp + len;
	uint64_t hval = FNV1_64_INIT;

	/* FNV-1a hash each octet of the buffer */
	while (bp < be)
		{
			/* xor the bottom with the current octet */
			hval ^= (uint64_t) * bp++;
			/* multiply by the 64 bit FNV magic prime mod 2^64 */
			hval *= FNV_64_PRIME;
		}
	return hval;
}

uint64_t FNVHashInt64(uint64_t hashme) {
	unsigned char buf[8];
	int i;

	for (i = 0; i < 8; i++)
		{
			buf[i] = (unsigned char) (hashme & 0x00000000000000FFULL);
			hashme >>= 8;
		}
	return FNVHash64(buf, 8);
}

uint64_t HashKeyToBucketNum(HashTable ht, uint64_t key)
{
	return key % ht->num_buckets;
}


static int find_key(LinkedList list, uint64_t key,
		    HTKeyValue *oldkeyvalue, int remove)
{
  LLIter iter;
  HTKeyValue *storage;
  uint64_t elements_in_bucket = NumElementsInLinkedList(list);

  if (elements_in_bucket == 0){
    return 0;
  }
  
  if ((iter = LLMakeIterator(list, 0)) == NULL) {
    return -1;
  }

  do {
    LLIteratorGetPayload(iter, (void **) &storage);
    if (storage->key == key) {
      /* key was found */
      oldkeyvalue->key = key;
      oldkeyvalue->value = storage->value;
      
      if (remove) {
	LLIteratorDelete(iter, &NullFree);
	//free(storage);
      }
      LLIteratorFree(iter);
      return 1;
    }
    
  } while(LLIteratorNext(iter));
  
  LLIteratorFree(iter);
  
  /* key not in given list */
  return 0;
}

int InsertHashTable(HashTable table, HTKeyValue newkeyvalue,
		    HTKeyValue *oldkeyvalue)
{
  assert(table);
  /* check if need to resize the table to make sure we get the right bucket */
  ResizeHashTable(table);
  uint64_t the_key = newkeyvalue.key;
  
  /* the bucket that will be home for the pair */
  uint64_t home_bucket;
  home_bucket = HashKeyToBucketNum(table, the_key);
  
  /* the list where the pair will be stored */
  LinkedList home_list;
  home_list = table->buckets[home_bucket];
  
  HTKeyValue *new_payload = malloc(sizeof(HTKeyValue));
  if(new_payload == NULL)
    return 0;
  new_payload->key = the_key;
  new_payload->value = newkeyvalue.value;
  
  if(NumElementsInLinkedList(home_list) == 0)
    {
      /* empty list, no need to search */
      PushLinkedList(home_list, new_payload);
      table->num_elements += 1;
      return 1;
    }
  
  /* need to check list for key */
  int found = find_key(home_list, the_key, oldkeyvalue, 1);
  if(found == -1) 			/* the out of memory code */
    {
      table->num_elements -=1;
      return 0;		
    }
  PushLinkedList(home_list, new_payload);
  
  if (found == 1){
    //     printf("%lu   %lu    found a copy \n", the_key, oldkeyvalue->key);
      return 2;
    }
  table->num_elements += 1;
  return 1;
}

int LookupHashTable(HashTable table, uint64_t key,
                    HTKeyValue *keyvalue)
{
	assert(table);
	uint64_t home_bucket = HashKeyToBucketNum(table, key);
	LinkedList home_list = table->buckets[home_bucket];
	return find_key(home_list, key, keyvalue, 0);
}

int RemoveFromHashTable(HashTable table, uint64_t key,
                        HTKeyValue *keyvalue)
{
	assert(table);
	uint64_t home_bucket = HashKeyToBucketNum(table, key);
	LinkedList home_list = table->buckets[home_bucket];
	int found = find_key(home_list, key, keyvalue, 1);
	if (found)
		table->num_elements -= 1;
	return found;
}

HTIter HashTableMakeIterator(HashTable table)
{
	assert(table);
	HTIter iter = malloc(sizeof(HTIterRecord));
	if (iter == NULL)
		return NULL;
	
	if (table->num_elements == 0)
		{

			iter->is_valid = false;
			iter->ht = table;
			iter->bucket_it = NULL;
			return iter;
		}

	iter->is_valid = true;
	iter->ht = table;
	uint32_t i;

	for (i = 0; i < table->num_buckets; i++)
		{
			if (NumElementsInLinkedList(table->buckets[i]) > 0)
				{
					iter->bucket_num = i;
					break;
				}
		}
	assert(i < table->num_buckets);
	iter->bucket_it = LLMakeIterator(table->buckets[iter->bucket_num], 0UL);
	if(iter->bucket_it == NULL)
		{
			free(iter);
			return NULL;
		}
	return iter;
}
static void ResizeHashTable(HashTable ht)
{
	/* resize if load factor > 3 */
	if (ht->num_elements < (3 * ht->num_buckets))
		return;

	/* resize case: allocate new hashtable & transfer payloads
	 *           from old ht to new ht and free old ht
	 */

	HashTable newht = AllocateHashTable(ht->num_buckets * 9);
	if (newht == NULL)
		return;

	HTIter it = HashTableMakeIterator(ht);
	if (it == NULL)
		{
			FreeHashTable(newht, &NullFree);
			return;
		}

	HTKeyValue item, dummy;
	printf("RESIZEINGIFDNGIFDNGIN\n");
	while (!HTIteratorPastEnd(it))
		{
			assert(HTIteratorGet(it, &item) == 1);
			if (InsertHashTable(newht, item, &dummy) != 1)
				{
					HTIteratorFree(it);
					FreeHashTable(newht, &NullFree);
					return;
				}
			HTIteratorNext(it);
		}

	HTIteratorFree(it);

	HashTableRecord tmp;

	tmp = *ht;
	*ht = *newht;
	*newht = tmp;
	FreeHashTable(newht, &NullFree);

	return;
}

void HTIteratorFree(HTIter iter)
{
	assert(iter);

	if (iter->bucket_it != NULL)
		LLIteratorFree(iter->bucket_it);
	
	free(iter);
}

int HTIteratorNext(HTIter iter)
{
  assert(iter);
  
  if (HTIteratorPastEnd(iter))
    return 0;
  
  //	uint64_t i, num_buckets;
  
  //	num_buckets = iter->ht->num_buckets;
  //	int found;
  
  /* case 1: still an element in current bucket */
  if (LLIteratorHasNext(iter->bucket_it))
    {
      LLIteratorNext(iter->bucket_it);
      //		found = 1;
      
      return 1;
    }
  /* case 2: iter is at the end of a list but not past the full table  */
  iter->bucket_num++;
  while(!HTIteratorPastEnd(iter))
    {
      
      if (NumElementsInLinkedList(iter->ht->buckets[iter->bucket_num]) > 0)
	{
	  LLIteratorFree(iter->bucket_it);
	  iter->bucket_it = LLMakeIterator(iter->ht->buckets[iter->bucket_num], 0);
	  if (iter->bucket_it == NULL)
	    {
	      iter->is_valid = false;
	      return 0;
	    }
	  return 1;
	}
      
      iter->bucket_num++;
    }
  
  return 0;
}

/* 
struct ht_itrec;
typedef struct ht_itrec *HTIter;  // same trick to hide implementation.

 
typedef struct htrec {
    uint64_t        num_buckets;   // # of buckets in this HT?
    uint64_t        num_elements;  // # of elements currently in this HT?
    LinkedList     *buckets;       // the array of buckets
} HashTableRecord;
typedef struct ht_itrec {
    bool       is_valid;    // is this iterator valid?
    HashTable  ht;          // the HT we're pointing into
    uint64_t   bucket_num;  // which bucket are we in?
    LLIter     bucket_it;   // iterator for the bucket, or NULL
} HTIterRecord;
 */

int HTIteratorPastEnd(HTIter iter)
{
	assert(iter);
	if (!iter->is_valid || iter->ht->num_elements == 0
		|| (iter->bucket_num >= iter->ht->num_buckets))
		{
			iter->is_valid = false;
			return 1;
		}

	return 0;
}

int HTIteratorGet(HTIter iter, HTKeyValue *keyvalue)
{
	assert(iter);
	
	if (HTIteratorPastEnd(iter))
		return 0;
	HTKeyValue *payload = malloc(sizeof(HTKeyValue));
	if (payload == NULL)
		return 0;
	LLIteratorGetPayload(iter->bucket_it, (void **) &payload);

	keyvalue->key = payload->key;
	keyvalue->value = payload->value;

	return 1;
}

void PrintHashTable(HashTable table)
{
  HTIter iter = HashTableMakeIterator(table);
  printf("This hash table has %lu entries in %lu bucks:\n",
	 table->num_elements, table->num_buckets);

  HTKeyValue copy;
  int cur_buck = iter->bucket_num;
  printf("[B=%d]-->", cur_buck);
  if (!HTIteratorPastEnd(iter)) {
    do
      {
	if (cur_buck != iter->bucket_num) {
	  cur_buck = iter->bucket_num;
	  printf("\n[B=%d]-->", cur_buck);
	}
	HTIteratorGet(iter, &copy);
	printf("[%lu,%p]", copy.key, copy.value);
	
      } while (HTIteratorNext(iter));
    
  }
  HTIteratorFree(iter);
}

int HTIteratorDelete(HTIter iter, HTKeyValue *keyvalue)
{
	assert(iter);

	HTKeyValue kv;
	int r, retval;

	r = HTIteratorGet(iter, &kv);
	if (r == 0)
		return 0;

	r = HTIteratorNext(iter);
	if (r == 0)
		retval = 2;
	else
		retval = 1;

	r = RemoveFromHashTable(iter->ht, kv.key, keyvalue);
	assert(r == 1);
	assert(kv.key == keyvalue->key);
	assert(kv.value == keyvalue->value);

	return retval;
}
 

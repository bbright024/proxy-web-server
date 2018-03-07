#include <stdio.h>

#include <includes/csapp.h>
#include <includes/cache.h>
#include <includes/LinkedList.h>
#include <includes/HashTable.h>
#include <includes/defs.h>
#include <includes/params.h>
#include <includes/errors.h>

/* Global shared variables */
HashTable h_table = NULL;
LinkedList ob_list = NULL;

static size_t cache_size;

static sem_t cache_size_mutex;
static sem_t cache_mutex;
static sem_t cache_table_mutex;

typedef struct cache_object{
  char *location;
  size_t size;
  char type[MAXLINE];
  char host[MAXLINE];
  char filename[MAXLINE];
} CacheOb;

/* Internal functions */
static void free_cache_ob(CacheOb *cp);
static int remove_cache_lru(size_t min_size);

/* free function that does nothing - for HashTable library*/
static void NullFree(void *freeme) { }

/* initialize the cache objects - must not be called more than once.  */
/* TODO: add a way to restart the cache by moving c_init to a global var */
int cache_init()
{
  static int c_init;
  if (c_init)
    return 0;

  c_init++;
    
  cache_size = 0;
  h_table = AllocateHashTable(NBUCKETS);
  if (!h_table)
    return -ENOMEM;
  ob_list = AllocateLinkedList();
  if (!ob_list) {
    FreeHashTable(h_table, (LLPayloadFreeFnPtr)NullFree); 
    return -ENOMEM;
  }
  if ((sem_init(&cache_mutex, 0, 1)) < 0 ||
      (sem_init(&cache_table_mutex, 0, 1)) < 0 ||
      (sem_init(&cache_size_mutex, 0, 1)) < 0)
    return -ENOMEM;

  return 0;
}

/* Frees all dynamically allocated parts of a CacheOb struct */
static void free_cache_ob(CacheOb *obp)
{
  if (!obp)
    return;
  free(obp->location);
  free(obp);
}

/* Internal function in case P and V need to be stay locked while the cache is destroyed */
static void __cache_destroy()
{
  FreeHashTable(h_table, (LLPayloadFreeFnPtr)NullFree); //the linked list will free everything
  FreeLinkedList(ob_list, (LLPayloadFreeFnPtr)free_cache_ob);

  V(&cache_table_mutex);
  V(&cache_mutex);

  // done to keep valgrind happy
  // when thread exits, semaphores get cleaned by kernel automatically
  Sem_destroy(&cache_table_mutex);
  Sem_destroy(&cache_mutex);
  Sem_destroy(&cache_size_mutex);
}

/* Frees everything dynamically allocated by the cache and cache_init 
 * cache_init must be called after this in order to use cache again.
 */
void cache_free_all()
{

  P(&cache_mutex);
  P(&cache_table_mutex);

  __cache_destroy();

}

/* 
 * Checks the cache hash table for the host/filename combo. 1 if present, 0 if not

int check_cache_table(uint64_t key)
{
  uint64_t file_and_host_hash(char *filename, char *host);
  HTKeyValue kvp;
  int r;
  r = LookupHashTable(h_table, key, &kvp);
  if ()
}
 */
/* 
 * Checks the cache for the host/filename combo.
 * Returns:
 *  - 0 if not found.
 *  - 1 if found - saves the type, object, and size as side-effects
 *  - -EINVAL if args are invalid
 */

int check_cache(char *host, char *filename, void *obj_buf, char *type, size_t *sizep)
{
  if (!host || !filename || !obj_buf || !sizep || !type)
    return -EINVAL;
  
  P(&cache_mutex);
  int n = NumElementsInLinkedList(ob_list);
  if (n <= 0) {
    V(&cache_mutex);
    return 0;
  }

  uint64_t key = file_and_host_hash(filename, host);
  HTKeyValue kvp;

  P(&cache_table_mutex);
  int r;
  r = LookupHashTable(h_table, key, &kvp);

  if (r != 1) {
    V(&cache_table_mutex);
    V(&cache_mutex);
    return 0;
  }
  CacheOb *obp = (CacheOb *)kvp.value;
  strncpy(type, obp->type, MAXLINE);
  *sizep = obp->size;
  memcpy(obj_buf, obp->location, obp->size);

  V(&cache_table_mutex);
  V(&cache_mutex);

  return 1;
  /* below is the old code for just LinkedList searching */
  //printf("\nSEARCHING:\n obj = %p \nhost = %s \nfile = %s \n", obj_buf, host, filename);
  /* 
  CacheOb *op;
  LLIter iter;
  if ((iter = LLMakeIterator(ob_list, 0)) == NULL) {
    fprintf(stderr, "make iter error in check_cache\n");
    V(&cache_table_mutex);
    V(&cache_mutex);
    return 0;
  }
   */
  /* loop through the cache until obj found or at tail */

  /* 
  do {
    LLIteratorGetPayload(iter, (void **)&op);
    if(!strcmp(op->filename, filename) && !strcmp(op->host, host)) {
      // copies object data from cache to specified buffers
      //      strncpy(type, op->type, MAXLINE);
      //      *sizep = op->size;
      //      memcpy(obj_buf, op->location, op->size);
      
      // file was accesed, so move to front (LRU)
      LLIteratorMoveToHead(iter);
      V(&cache_table_mutex);
      V(&cache_mutex);
      LLIteratorFree(iter);
      
      return 1;
    }
  }  while (LLIteratorNext(iter));
    */

  V(&cache_table_mutex);
  V(&cache_mutex);
  //  LLIteratorFree(iter);
  //  printf("did not find the file\n");
  return 0;
}


static CacheOb *new_cache_obj(void *object, size_t size, char *host, char *filename, char *type)
{
  if (!object || !host || !filename || !size || !type)  {
    return NULL;
  }  
  if (strlen(host) > MAXLINE || strlen(filename) > MAXLINE ||
      size > MAX_OBJECT_SIZE || strlen(type) > MAXLINE) {
    return NULL;    
  }

  CacheOb *obp = malloc(sizeof(CacheOb));
  if (!obp) {
    return NULL;
  }
  memset(obp, 0, sizeof(CacheOb));
  if ((obp->location = malloc(size)) == NULL) {
    free(obp);
    return NULL;
  }

  /* store the metadata in the CacheOb struct */
  obp->size = size;
  memcpy(obp->location, object, size);
  strncpy(obp->host, host, MAXLINE);
  strncpy(obp->filename, filename, MAXLINE);
  strncpy(obp->type, type, MAXLINE);

  return obp;
}


int add_to_cache_table(CacheOb *obp)
{
  
  if (!obp)
    return -E_NO_SPACE;

  /* TODO: fix how HTKeyValue has to be passed by value to insert hash table */
  HTKeyValue new_kv, old_kv;

  new_kv.key = file_and_host_hash(obp->filename, obp->host);
  new_kv.value = obp;
  
  int r = InsertHashTable(h_table, new_kv, &old_kv);
  if (r == 0){
    return -ENOMEM;
  }
  else if (r == 2) { // a key collision shouldn't affect the overall program at all since the old one gets replaced
    fprintf(stderr, "error - key collision - shouldn't happen!\n");
    free(old_kv.value);
  }

  //  PrintHashTable(h_table);
  
  return r;
}

static inline void lock_all_cache()
{
  P(&cache_mutex);
  P(&cache_table_mutex);
  P(&cache_size_mutex);
}

static inline void unlock_all_cache()
{
      V(&cache_size_mutex);
      V(&cache_table_mutex);
      V(&cache_mutex);
}
/* Adds an object to the cache.
 * - removes the last obj on linked list if cache is full
 * - pushes new obj to front
 * - locks access to the ll and cache_size
 * - fails if size is above MAX_OBJECT_SIZE, or if any parameters are invalid.
 * - uses LLIterator to delete from end of list if needed (SliceLinked needs payloadptr)
 *
 * Returns 0 on success, -ENOMEM or -E_NO_SPACE or -3 on other errors
 */

int add_to_cache(void *object, size_t size, char *host, char *filename, char *type)
{
  CacheOb *obp = new_cache_obj(object, size, host, filename, type);
  if (!obp) {
    return -ENOMEM;
  }
  
  lock_all_cache();
  
  if (cache_size + size > MAX_CACHE_SIZE) {
    /* need to free oldest obj in list that leaves enough space*/
    /* TODO: remove from cache_table as well */
    if(remove_cache_lru(size) != 0) {
      /* if here, must be an error in the earlier space checks */
      free(obp->location);
      free(obp);
      unlock_all_cache();
      return -E_NO_SPACE;
    }
  }
  
  if (add_to_cache_table(obp) < 0 ||
      !PushLinkedList(ob_list, obp)) {
    unlock_all_cache();
    thread_unix_error("couldn't add to cache");
  }
    
  cache_size += size;
  unlock_all_cache();
  return 0;
}

/* Search backwards from the tail of the cache, removing the first
 * object of smaller size than the given parameter.  
 *   -reads and writes the cache, must be locked before called
 *   -ALL LOCKS SHOULD BE HELD ALREADY
 * Returns 0 if successful, negative on error. */
static int remove_cache_lru(size_t min_size)
{
  LLIter iter;
  if ((iter = LLMakeIterator(ob_list, 1)) == NULL) {
    fprintf(stderr, "make iter error: remove_cache_lru");
    return -1;
  }
  CacheOb *obp_r;
  while (1) {
    LLIteratorGetPayload(iter, (void **)&obp_r);
    if (min_size < obp_r->size)
      break;
    if(!LLIteratorPrev(iter)){
      // logically, this should never happen because it can only happen if the size of the obj was
      // too big to ever be cached.  
      LLIteratorFree(iter);
      return -1;
    }
  }
  cache_size -= obp_r->size;

  HTKeyValue old_kv;
  RemoveFromHashTable(h_table, file_and_host_hash(obp_r->filename, obp_r->host), &old_kv);
  LLIteratorDelete(iter, (LLPayloadFreeFnPtr)free_cache_ob);

  LLIteratorFree(iter);
  return 0;
}

/* prints out detailed info on the current cache state.
 * passing in HUMAN_READABLE translates bytes to kilobytes  */
void print_cache(int human)
{
  CacheOb *op;
  int n, i = 0;
  LLIter iter;
  P(&cache_mutex);
  if (!ob_list|| (n = NumElementsInLinkedList(ob_list)) <= 0) {
    printf("\nThe cache is empty.\n");
    V(&cache_mutex);
    return;
  }

  printf("\nThere is/are %d object(s) in the cache.\n", n);

  P(&cache_size_mutex);
  if(human) {
    printf("Total cache size (kilobytes): %zu", cache_size >> 10);
  }
  else
    printf("Total cache size (bytes): %zu", cache_size);

  V(&cache_size_mutex);
  if((iter = LLMakeIterator(ob_list, 0)) == NULL){
    fprintf(stderr, "make iter error\n");
    V(&cache_mutex);
    return;
  };
  
  do
    {
      printf("Object %d of %d:\n", i + 1, n);
      LLIteratorGetPayload(iter, (void **)&op);

      printf("Host: %s\nFile: %s\nType: %s\n", op->host, op->filename, op->type);
      if (human) {
	printf("Size (in kilobytes): %zu\n", op->size >> 10);
      }
      else
	printf("Size (in bytes): %zu\n", op->size);
      i++;
    } while (LLIteratorNext(iter));

  P(&cache_table_mutex);
  PrintHashTable(h_table);
  V(&cache_table_mutex);
  V(&cache_mutex);
  // always free AFTER releasing lock; reduces time in lock 
  LLIteratorFree(iter); 	
  return;
}


/* helper function that concatenates the filename and hostname
 * for FNVHash, returns the hash itself.  
  */
uint64_t file_and_host_hash(char *filename, char *host)
{
  unsigned int combo = MAXLINE * 2;
  char buf[combo];
  strncpy(buf, filename, MAXLINE);
  strncat(buf, host, MAXLINE);

  return FNVHash64((unsigned char *)buf, strlen(buf));
}

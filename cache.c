#include <stdio.h>

#include <includes/csapp.h>
#include <includes/cache.h>
#include <includes/LinkedList.h>

#define E_NO_MEM 1
#define E_NO_SPACE 2

/* Global shared variables */
LinkedList ob_list;
static size_t cache_size;
static sem_t cache_mutex;

/* Keep CacheOb in here so not seen by other programs */
typedef struct {
  char *location;
  size_t size;
  char type[MAXLINE];
  char host[MAXLINE];
  char filename[MAXLINE];
} CacheOb;

/* Internal functions */
void free_cache_ob(CacheOb *cp);
static int remove_cache_lru(size_t min_size);

/* initialize the cache objects - must not be called more than once.
 * 
 * Returns 0 on success, -E_NO_MEM on failure
 */
int cache_init()
{
  cache_size = 0;
  ob_list = AllocateLinkedList();
  sem_init(&cache_mutex, 0, 1);
  if (!ob_list)
    return -E_NO_MEM;
  return 0;
}

/* Frees all parts of a CacheOb struct */
void free_cache_ob(CacheOb *cp)
{
  if (!cp)
    return;
  free(cp->location);
  free(cp);
}

/* Frees everything in the cache. 
 * cache_init must be called after this in order to use cache again.
 */
void cache_free_all() {
  FreeLinkedList(ob_list, (LLPayloadFreeFnPtr)free_cache_ob);
}

/* 
 * Checks the cache for the host/filename combo.
 * Returns:
 *  - 0 if not found.
 *  - 1 if found - saves the type, object, and size as sideaffects
 */
int check_cache(char *host, char *filename, void *obj_buf, char *type, size_t *sizep)
{
  P(&cache_mutex);
  int n = NumElementsInLinkedList(ob_list);
  if (n <= 0) {
    V(&cache_mutex);
    return 0;
  }
    
  //printf("\nSEARCHING:\n obj = %p \nhost = %s \nfile = %s \n", obj_buf, host, filename);  
  CacheOb *op;
  LLIter iter;
  if ((iter = LLMakeIterator(ob_list, 0)) == NULL) {
    fprintf(stderr, "make iter error in check_cache\n");
    V(&cache_mutex);
    return 0;
  }
  /* loop through the cache until obj found or at tail */
  do {
    LLIteratorGetPayload(iter, (void **)&op);
    if(!strcmp(op->filename, filename) && !strcmp(op->host, host)) {
      /* copies object data from cache to specified buffers */
      strncpy(type, op->type, MAXLINE);
      *sizep = op->size;
      memcpy(obj_buf, op->location, op->size);
      
      /* file was accesed, so move to front (LRU) */
      LLIteratorMoveToHead(iter); 
      V(&cache_mutex);
      LLIteratorFree(iter);
      return 1;
    }
  }  while (LLIteratorNext(iter));

  V(&cache_mutex);
  LLIteratorFree(iter);
  printf("did not find the file\n");
  return 0;
}

/* Adds an object to the cache.
 * - removes the last obj on linked list if cache is full
 * - pushes new obj to front
 * - locks access to the ll and cache_size
 * - fails if size is above MAX_OBJECT_SIZE, or if any parameters are invalid.
 * - uses LLIterator to delete from end of list if needed (SliceLinked needs payloadptr)
 *
 * Returns 0 on success, -E_NO_MEM or -E_NO_SPACE or -3 on other errors
 */
int add_to_cache(void *object, size_t size, char *host, char *filename, char *type)
{

  if (!object || !host || !filename || !size || !type)  {
    return -E_NO_SPACE;
  }  
  if (strlen(host) > MAXLINE || strlen(filename) > MAXLINE ||
      size > MAX_OBJECT_SIZE || strlen(type) > MAXLINE)
    return -E_NO_SPACE;

  CacheOb *obp;
  obp = malloc(sizeof(CacheOb));
  memset(obp, 0, sizeof(CacheOb));
  if (!obp)
    return -E_NO_MEM;
  /* allocate 'size' space for the object*/
  obp->location = malloc(size);
  if (!obp->location) {
    free(obp);
    return -E_NO_MEM;
  }

  /* store the metadata in the CacheOb struct */
  obp->size = size;
  memcpy(obp->location, object, size);
  strncpy(obp->host, host, MAXLINE);
  strncpy(obp->filename, filename, MAXLINE);
  strncpy(obp->type, type, MAXLINE);
  P(&cache_mutex);
  if (cache_size + size > MAX_CACHE_SIZE) {
    /* need to free oldest obj in list that leaves enough space*/
    if(remove_cache_lru(size) != 0) {
      /* if here, must be an error in the earlier space checks */
      free(obp->location);
      free(obp);
      V(&cache_mutex);
      return -E_NO_SPACE;
    }
  }

  PushLinkedList(ob_list, obp);
  cache_size += size;

  /* unlock access here */
  V(&cache_mutex);
  printf("successfully added to cache\n");

  return 0;
}

/* Search backwards from the tail of the cache, removing the first
 * object of smaller size than the given parameter.  
 *   -reads and writes the cache, must be locked before called
 *   
 * Returns 0 if successful, negative on error. */
static int remove_cache_lru(size_t min_size)
{
  LLIter iter;
  if ((iter = LLMakeIterator(ob_list, 1)) == NULL) {
    fprintf(stderr, "make iter error: remove_cache_lru");
    
  }
    CacheOb *obp_r;
    while (1) {
      LLIteratorGetPayload(iter, (void **)&obp_r);
      if (min_size < obp_r->size)
	break;
      if(!LLIteratorPrev(iter)){ /* at head of list if false */
	LLIteratorFree(iter);
	return -1;
      }
    } 
    cache_size -= obp_r->size;

    if(!LLIteratorDelete(iter, (LLPayloadFreeFnPtr)free_cache_ob)) {
      /* if here, the cache is now empty */
      LLIteratorFree(iter);
      fprintf(stderr, "cache is empty\n");
      return -3;
    }
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
  if (!ob_list) {
    printf("\nThe cache is empty.\n");
    V(&cache_mutex);
    return;
  }
  n = NumElementsInLinkedList(ob_list);


  
  if (n <= 0) {
    printf("\nThe cache is empty.\n");
    V(&cache_mutex);
    return;
  }
  else if (n == 1) {
      printf("\nThere is 1 object in the cache.\n");
  }
  else 
    printf("\nThere are %d objects in the cache.\n", n);

  if(human) {
    printf("Total cache size (kilobytes): %zu", cache_size >> 10);
  }
  else
    printf("Total cache size (bytes): %zu", cache_size);


  if((iter = LLMakeIterator(ob_list, 0)) == NULL){
    fprintf(stderr, "make iter error\n");
    V(&cache_mutex);
    return;
  };
  
  do
    {
      printf("Object %d of %d:\n", i, n);
      LLIteratorGetPayload(iter, (void **)&op);

      printf("Host: %s\nFile: %s\nType: %s\n", op->host, op->filename, op->type);
      if (human) {
	printf("Size (in kilobytes): %zu\n", op->size >> 10);
      }
      else
	printf("Size (in bytes): %zu\n", op->size);
      i++;
    } while (LLIteratorNext(iter));
  
  V(&cache_mutex);
  /* always free AFTER releasing lock; reduces time lock spent 
   * in this thread. */
  LLIteratorFree(iter); 	
  return;
}


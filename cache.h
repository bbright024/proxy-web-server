#include <stdbool.h>
/* 
 * Sets up the free list of open cache space
 * and the storage location for each cache object
 */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

int cache_init();
int add_to_cache(void *object, size_t size, char *host, char *filename, char *type);
int check_cache(char *host, char *filename, void *obj_buf, char *type, size_t *size_buf);
void cache_free_all();
void print_cache(int human);


/* zero out the least recently used object in the cache 
 * and place it in free list */
//void evict_object();

/* copies object into the cache. returns 0 on success,
 * negative if size is invalid or object null. */
//int set_object(void *object, size_t size);


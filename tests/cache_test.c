#include <stdlib.h>
#include <stdio.h>
#include <includes/LinkedList.h>
#include <includes/HashTable.h>
#include <includes/csapp.h>
#include <includes/cache.h>
#include <includes/proxy.h>
#include <includes/http.h>
#include <includes/sys_wraps.h>
#include "minunit.h"


int test_new_cache_obj();
typedef struct test_struct {
  char buf[MAXLINE]
};

//int cache_init();
int test_cache_init() {
  int r;
  r = cache_init();
  r += cache_init();
  if (r)
    return 1;

  return 0;
}

//void cache_free_all();
int test_cache_free_all() {
  cache_free_all();
  cache_free_all();
  return 0;
}

//int check_cache(char *host, char *filename, void *obj_buf, char *type, size_t *size_buf);
int test_check_cache() {
  return 0;
}

//int add_to_cache_table(CacheOb *obp)
int test_add_to_cache_table() {
  return 0;
}

//int add_to_cache(void *object, size_t size, char *host, char *filename, char *type);
int test_add_to_cache() {
  return 0;
}

//void print_cache(int human);
int test_print_cache() {
  return 0;
}

//uint64_t file_and_host_hash(char *filename, char *host);
int test_file_and_host_hash() {
  return 0;
}


/* 
typedef struct cache_object{
  char *location;
  size_t size;
  char type[MAXLINE];
  char host[MAXLINE];
  char filename[MAXLINE];
} CacheOb;
 */


int main(int argc, char *argv[])
{
  int ret = 0;
  
  printf("Testing cache.c functions...\n");

  ret += test_cache_init();
  ret += test_cache_free_all();


  cache_init();

  ret += test_new_cache_obj();

  ret += test_cache_free_all();

  ret += test_cache_init();
  
  // CacheOb *new_cache_obj(void *object, size_t size, char *host, char *filename, char *type)
  //now build up some testable objects in the cache

  struct test_struct t = {0};
  CacheOb *test_ob = NULL;
  size_t size = sizeof(struct test_struct);
  char *host = "foo";
  char *filename = "bar";
  char *type = "baz";
  
  
  ret += test_check_cache();
  ret += test_add_to_cache_table();
  ret += test_add_to_cache();
  ret += test_print_cache();
  ret += test_file_and_host_hash();

  cache_free_all();

  printf("# of test failures: %d\n", ret);

  return 0;
}




int test_new_cache_obj() {
  struct test_struct t = {0};
  CacheOb *test_ob = NULL;
  size_t size = sizeof(struct test_struct);
  char *host = "foo";
  char *filename = "bar";
  char *type = "baz";
  char *badhost = malloc(2 * MAXLINE);
  char *badfile = malloc(2 * MAXLINE);
  char *badtype = malloc(2 * MAXLINE);
  memset(badhost, 'a', MAXLINE * 2);
  memset(badfile, 'b', MAXLINE * 2);
  memset(badtype, 'c', MAXLINE * 2);
  char *func_string = "test_new_cache_obj failed: ";
  test_ob = new_cache_obj(NULL, NULL, NULL, NULL, NULL);
  if (test_ob) {
    printf("%s all null fields test_obj", func_string);
    return 1;
  }
    
  test_ob = new_cache_obj((void *)&t, size, badhost, filename, type);
  if (test_ob) {
    printf("%s badhost field", func_string);
    return 1;
  }
  
  test_ob = new_cache_obj((void *)&t, size, host, badfile, type);
  if (test_ob) {
    printf("%s badfile field", func_string);
    return 1;
  }
  test_ob = new_cache_obj((void *)&t, size, host, filename, badtype);
  if (test_ob) {
    printf("%s badtype field", func_string);
    return 1;
  }

  test_ob = new_cache_obj((void *)&t, MAX_CACHE_SIZE * 2, host, filename, type);
  if (test_ob) {
    printf("%s bad size - max cache worked", func_string);
    return 1;
  }
  test_ob = new_cache_obj((void *)&t, MAX_OBJECT_SIZE * 2, host, filename, NULL);
  if (test_ob) {
    printf("%s bad size - max obj worked", func_string);
    return 1;
  }

  test_ob = new_cache_obj((void *)&t, size, host, filename, type);
  if (!test_ob) {
    printf("%s proper try failed", func_string);
    return 1;
  }

  free(badtype);
  free(badhost);
  free(badfile);
  return 0;
  
}

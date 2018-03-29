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

#define C_MAX_ELEMENTS (MAX_CACHE_SIZE / MAX_OBJECT_SIZE)

int test_new_cache_obj();
typedef struct test_struct {
  char buf[MAXLINE];
} TestC;

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
  cache_init();
  int i;
  CacheOb *test_c;
  char *filename;
  add_to_cache_table(NULL);

  for (i = 0; i < MAX_OBJECTS_MAX_SIZE + 5; i++) {
    filename = malloc(1);
    *filename = 'a' + i;
    test_c = malloc(sizeof(CacheOb));
    test_c->filename[0] = *filename;
    test_c->host[0] = 'b' + i;
    test_c->size = MAX_OBJECT_SIZE;
    add_to_cache_table(test_c);
    
  }

  cache_free_all();
  return 0;
  
}

//int add_to_cache(void *object, size_t size, char *host, char *filename, char *type);
int test_add_to_cache() {
  int r;
  r = add_to_cache(NULL, NULL, NULL, NULL, NULL);
  if (!r)
    return 1;

  cache_free_all();
  char *asdf = "asdf";
  r = add_to_cache((void *) asdf, 1, asdf, asdf, asdf);
  if (!r)
    return 1;
  
  cache_init();
  //TestC *test_c;
  //  int i;
  //  char *filename;

  char *filename;
  void *buf = malloc(MAX_OBJECT_SIZE);
  int i;
  for (i = 0; i < MAX_OBJECTS_MAX_SIZE + 5; i++) {
      filename = malloc(1);
      *filename = 'a' + i;
      add_to_cache(buf, MAX_OBJECT_SIZE, "a", filename, "t");
      free(filename);
      
  }

  /* 
  for (i = 0; i < C_MAX_ELEMENTS + 2; i++) {
    test_c = malloc(sizeof(struct test_struct));
    filename = 'a' + (i % 128);
    add_to_cache((void *)test_c, MAX_OBJECT_SIZE, filename, asdf, asdf);
    
  }
     */

  //  add_to_cache((void *)test_c, MAX_OBJECT_SIZE, asdf, asdf, asdf);
  cache_free_all();
  
  return 0;
}

//void print_cache(int human);
int test_print_cache() {
  cache_free_all();
  print_cache(1);
  print_cache(0);
  
  cache_init();
  print_cache(1);
  print_cache(0);

  TestC *test_c = malloc(sizeof(struct test_struct));
  add_to_cache((void *)test_c, sizeof(struct test_struct), "asdf", "asdf", "asdf" );
  print_cache(1);
  print_cache(0);
  cache_free_all();
  
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

  ret += test_check_cache();
  ret += test_add_to_cache_table();
  ret += test_add_to_cache();
  ret += test_print_cache();
  ret += test_file_and_host_hash();


  print_cache(1);
  print_cache(0);

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

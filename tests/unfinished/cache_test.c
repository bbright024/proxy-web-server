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

//int cache_init();
int test_cache_init() {
  return 0;
}

//void cache_free_all();
int test_cache_free_all() {
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

int main(int argc, char *argv[])
{
  int ret = 0;
  
  printf("Testing cache.c functions...\n");

  ret += test_cache_init();
  ret += test_check_cache();
  ret += test_add_to_cache_table();
  ret += test_add_to_cache();
  ret += test_print_cache();
  ret += test_file_and_host_hash();
  ret += test_cache_free_all();

  printf("# of test failures: %d\n", ret);

  return 0;
}

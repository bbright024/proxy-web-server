#include <stdlib.h>
#include <stdio.h>
#include <includes/LinkedList.h>
#include <includes/HashTable.h>
#include <includes/csapp.h>
#include <includes/cache.h>
#include <includes/proxy.h>
#include <includes/http.h>
#include "minunit.h"


int test_cache()
{
  return add_to_cache_table(NULL);
  
}

int test_http()
{
  return http_read_request_line(NULL, NULL);
}

int main(int argc, char *argv[])
{
  int ret = 0;
  
  printf("Testing functions...\n");

  ret += test_cache();
  ret += test_http();

  printf("# of test failures: %d\n", ret);

  return 0;
}

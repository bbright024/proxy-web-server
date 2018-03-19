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



//void run_proxy(char *proxy_port);
int test_run_proxy() {
  return 0;
}


int main(int argc, char *argv[])
{
  int ret = 0;
  
  printf("Testing functions...\n");

  ret += test_run_proxy();
  //  ret += test_();
  //  ret += test_();
  //  ret += test_();

  printf("# of test failures: %d\n", ret);

  return 0;
}

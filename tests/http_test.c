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


//const char *http_read_request_line(rio_t *rio, ReqData *req_d)
int test_http_read_request_line()
{
  return http_read_request_line(NULL, NULL);
}

//const char *http_write_request(int destfd, ReqData *req_d)
int test_http_write_request()
{
  return 0;
}

//const char *http_relay_resp_body(int connfd, rio_t *rio_dest, int size, void *obj_buf)
int test_http_relay_resp_body()
{
  return 0;
}
//const char *http_relay_resp_headers(int connfd, ReqData *req_d,
//				    rio_t *rio_dest, int *size, char *type)
int test_http_relay_resp_headers()
{
  return 0;
}

//void http_serve(int connfd, void *obj_buf, int size, char *type)
int test_http_serve()
{
  return 0;
}
//void http_err(int fd, char *cause, char *errnum,
//		 char *shortmsg, const char *longmsg)
int test_http_err()
{
  return 0;
}

//int url_parse(ReqData *req_d, char *url)
int test_url_parse()
{
  return 0;
}


int main(int argc, char *argv[])
{
  int ret = 0;
  
  printf("Testing http.c functions...\n");

  ret += test_http_read_request_line();
  ret += test_http_write_request();
  ret += test_http_relay_resp_body();
  ret += test_http_relay_resp_headers();
  ret += test_http_serve();
  ret += test_http_err();
  ret += test_url_parse();
  printf("# of test failures: %d\n", ret);

  return 0;
}

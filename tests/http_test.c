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
void const_test_reqdata(ReqData *r);

//const char *http_read_request_line(rio_t *rio, ReqData *req_d)
int test_http_read_request_line()
{
  
  http_read_request_line(NULL, NULL);
  return 0;
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
  ReqData *test_req = malloc(sizeof(ReqData));
  
  if(url_parse(NULL, NULL) == 0)
    return 1;

  if(url_parse(test_req, "asdfasdfasdf"))
    return 1;
  print_reqdata(test_req);
  memset(test_req, 0, sizeof(ReqData));

  if(url_parse(test_req, "asdfasdf:asdfasdf:asdfasdf:asdfasdf"))
    return 1;
  print_reqdata(test_req);
  memset(test_req, 0, sizeof(ReqData));

  if(url_parse(test_req, "GET asdfasdf:asdfasdf HTTP/1.0\r\n"))
    return 1;
  print_reqdata(test_req);
  memset(test_req, 0, sizeof(ReqData));

  if(url_parse(test_req, "www.asdf.com:123/asdf.php"))
    return 1;
  
  print_reqdata(test_req);
  memset(test_req, 0, sizeof(ReqData));
  
  free(test_req);
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
void print_reqdata(ReqData *r) {
  printf("method: %s\nurl: %s\nversion: %s\nhost: %s\nfilename: %s\ndest_port: %s\nheaders: %s\n",
	 r->method, r->url, r->version, r->host, r->filename, r->dest_port, r->headers);
  
}

void const_test_reqdata(ReqData *r) {
  strcpy(r->method, "method test");
  strcpy(r->url, "url test");
  strcpy(r->version, "version test");
  strcpy(r->host, "host test");
  strcpy(r->filename, "filename test");
  strcpy(r->dest_port, "dest_port test");
  strcpy(r->headers, "headers");
}

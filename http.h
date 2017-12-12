#pragma once

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include "csapp.h"

#define INVALID_URL 1

/* ReqData structs contain the information sent to the server. */
typedef struct {
  char method[MAXLINE];	       	/* GET or POST, e.g. */
  char url[MAXLINE];		/* full URL following method */
  char version[MAXLINE];	/* HTTP/1.1 or HTTP/1.0 */
  char host[MAXLINE];		/* Address of server host */
  char filename[MAXLINE];	/* requested filename on host*/
  char dest_port[MAXLINE];	/* requested port to connect (default 80)*/
  char headers[MAXLINE]; 	/* for any extra header lines */
} ReqData;

//enum meths {
//  GET = 1,
//  POST = 2,
//};

//enum vers {
//  HTTPv10 = 1,
//  HTTPv11 = 2,
//};




/* Read the request line like "GET /xxx HTTP/1.0" 
 *  - return Error message, or NULL if succeed
 */
const char *http_read_request_line(rio_t *rio, ReqData *req_d);

/* Write the request line & headers to next hop server. 
 *  - return Error message, or NULL if succeed
 */
const char *http_write_request(int destfd, ReqData *req_d);
/* Read all HTTP request headers.
 *  - return Error message, or NULL if succeed.
 */
const char *http_read_request_headers(rio_t *rp, ReqData *req_d);

const char *http_relay_resp_body(int connfd, rio_t *rio, int size, void *obj_buf);

/* Read the response HTTP headers from the upstream server 
 * returns NULL on success, or error message.
 */
const char *http_relay_resp_headers(int connfd, rio_t *rio_dest, int *size, char *type);
  
/* Send an HTTP error message. */
void http_err(int fd, char *cause, char *errnum, char *shortmsg, const char *longmsg);

/* Dispatcher for generating an HTTP response */
void http_serve(int fd, void *obj_buf, int size, char *type);

void http_serve_none(int fd, const char *);

/* URL decoder */
int url_parse(ReqData *req_d, char *url);


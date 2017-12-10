#pragma once

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include "csapp.h"

typedef struct {
  char method[MAXLINE];
  char url[MAXLINE];
  char version[MAXLINE];
  char host[MAXLINE];
  char filename[MAXLINE];
  char dest_port[MAXLINE];
} ReqData;

#define INVALID_URL 1

/* Read the request line like "GET /xxx HTTP/1.0" 
 *  - return Error message, or NULL if succeed
 */
const char *http_request_line(rio_t *rio, ReqData *req_d);

/* Read all HTTP request headers.
 *  - return Error message, or NULL if succeed.
 */
const char *htp_request_headers(int fd);

/* Send an HTTP error message. */
void http_err(int fd, int code, char *fmt, ...);

/* Dispatcher for generating an HTTP response */
void http_serve(int fd, const char *);

void http_serve_none(int fd, const char *);

/* URL decoder */
int url_parse(ReqData *req_d, char *url);


#include "http.h"
#include <sys/param.h>
#ifndef BSD
#include <sys/sendfile.h>
#endif
#include <sys/uio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Read the request line like "GET /xxx HTTP/1.0" 
 *  - return Error message, or NULL if succeed
 */
const char *http_request_line(int fd, char *reqpath, char *env, size_t *env_len);

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
void url_decode(char *dst, const char *src);

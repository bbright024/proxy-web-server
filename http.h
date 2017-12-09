#pragma once

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>

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


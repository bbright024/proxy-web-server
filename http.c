#include "http.h"
#include "csapp.h"
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
const char *http_request_line(rio_t *rio, ReqData *req_d)
{
  char buf[MAXLINE];
  Rio_readlineb(rio, buf, MAXLINE);
  sscanf(buf,  "%s %s %s", req_d->method, req_d->url, req_d->version);

  if (!req_d->method || !req_d->url || !req_d->version)
    return "Cannot parse HTTP request";

  if (strcmp(req_d->method, "GET") && strcmp(req_d->method, "POST"))
    return "Unsupported request (not GET or POST)";

  if (strstr(req_d->url, "https://"))
    return "This proxy does not support HTTPS";

  if (strcmp(req_d->version, "HTTP/1.1") &&
      strcmp(req_d->version, "HTTP/1.0"))
    return "Unsupported server protocol (not HTTP/1.1 or 1.0)";

  if ((url_parse(req_d, req_d->url)))
    return "Cannot parse URL request";
  
  return NULL;
}

/* Read all HTTP request headers.
 *  - return Error message, or NULL if succeed.
 */
const char *http_request_headers(int fd);

/* Send an HTTP error message. */
void http_err(int fd, int errnum, char *fmt, ...)
{
  char buf[MAXLINE], body[MAXBUF];

  /* build & print the HTTP response headers*/
  sprintf(buf, "HTTP/1.0 %d Error\r\n", errnum);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));

  char *msg = 0;
  va_list ap;
  va_start(ap, fmt);
  vasprintf(&msg, fmt, ap);
  va_end(ap);

  /* build the HTTP response body */
  sprintf(body, "<html><title>Proxy Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%d: Error\r\n", body, errnum);
  sprintf(body, "%s<p>%s\r\n", body, msg);
  sprintf(body, "%s<hr><em>The Bright Proxy</em>\r\n", body);
  Rio_writen(fd, body, strlen(body));

  free(msg);  
}

/* URL decoder */
int url_parse(ReqData *req_d, char *url)
{
  char *host_pos = req_d->host;
  char *file_pos = req_d->filename;
  char *port_pos = req_d->dest_port;
  char *pos = url;

  const char *http = "http://";
  int l = strlen(http);
  
  if (!strncmp(url, http, l)) {
    pos += l;
  }

  if (!(*pos) || (*pos == ' ') || (*pos == '/') || (*pos == '\r'))
    return -INVALID_URL;

  /* copy the host name */
  while (*pos && (*pos != ' ') && (*pos != '/') && (*pos != ':')
	 && (*pos != '\r')){
    *host_pos = *pos;
    pos++;
    host_pos++;
  }
  *host_pos = '\0';
  
  /* copy in the port, or set to 80 if none found */
  if (*pos == ':') {
    pos++;
    while (*pos && (*pos != '/') && (*pos != ' ') && (*pos != '\r')) {
      *port_pos = *pos;
      pos++;
      port_pos++;
    }
    *port_pos = '\0';
  }
  else
    strcpy(port_pos, "80");

  /* now pos is at a / or a space, if / copy the URI */
  if (*pos == '/') {
      while (*pos && (*pos != ' ') && (*pos != '\r')) {
	*file_pos = *pos;
	pos++;
	file_pos++;
      }
      *file_pos = '\0';
  }
  else 				/* no URI found, use default of / */
    strcpy(file_pos, "/");

  return 0;
}

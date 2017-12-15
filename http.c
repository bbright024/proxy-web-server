#include <includes/http.h>
#include <includes/csapp.h>

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

/* Strings for response/request headers */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/100.3\r\n";
static const char *connection_close = "Connection: close\r\n";
static const char *proxy_con_close = "Proxy-Connection: close\r\n";

static inline void add_crlfnull(char *buf, int l);
  
/* Read the request line like "GET /xxx HTTP/1.0" 
 *  - return Error message, or NULL if succeed
 */
const char *http_read_request_line(rio_t *rio, ReqData *req_d)
{
  char buf[MAXLINE];
  Rio_readlineb(rio, buf, MAXLINE);
  sscanf(buf,  "%s %s %s", req_d->method, req_d->url, req_d->version);

  if (!req_d->method || !req_d->url)
    return "Cannot parse HTTP request";

  if (strcmp(req_d->method, "GET")) //&& strcmp(req_d->method, "POST"))
    return "Unsupported request (not GET)";// or POST)";

  if (strstr(req_d->url, "https://"))

    return "This proxy does not support HTTPS";

  if (!req_d->version) {
    strcpy(req_d->version, "HTTP/0.9");
  }
  else if (strcmp(req_d->version, "HTTP/1.1") &&
      strcmp(req_d->version, "HTTP/1.0"))
    return "Unsupported server protocol (not HTTP/1.1 or 1.0)";

  if ((url_parse(req_d, req_d->url)))
    return "Cannot parse URL request";
  
  return NULL;
}

/* Write a request line + headers to next hop server. 
 *  - currently, only HTTP/1.0 is supported. 
 *  - returns NULL on success, err message on fail
 */
const char *http_write_request(int destfd, ReqData *req_d)
{
  /* rare corner - if snprintf reaches MAXBUF the final \r\n wont be added */
  char dest_buf[MAXBUF];
  /* Gotta do a rio_write after each user-provided string, protect from overflow. */
  snprintf(dest_buf, MAXBUF, "%s %s HTTP/1.0\r\n", req_d->method, req_d->filename);
  add_crlfnull(dest_buf, MAXBUF);
  Rio_writen(destfd, dest_buf, strlen(dest_buf));
  snprintf(dest_buf, MAXBUF, "Host: %s\r\n", req_d->host);
  add_crlfnull(dest_buf, MAXBUF);
  Rio_writen(destfd, dest_buf, strlen(dest_buf));
  /* TODO: add in all other user-provided request headers here */

  snprintf(dest_buf, MAXBUF, "%s%s%s\r\n", user_agent_hdr, connection_close,
	   proxy_con_close);
  add_crlfnull(dest_buf, MAXBUF);
  Rio_writen(destfd, dest_buf, strlen(dest_buf));

  return NULL;
}
/* Read all HTTP request headers.
 *  - return Error message, or NULL if succeed.
 */
const char *http_read_request_headers(rio_t *rp, ReqData *req_d)
{
  char buf[MAXLINE];
  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return NULL;
}

/* read the body of the response into the obj_buf */
const char *http_relay_resp_body(int connfd, rio_t *rio_dest, int size, void *obj_buf)
{
  char *p = NULL;
  p = (char *)obj_buf;
  int n;

  /* proxy puts the file into body while # of bytes read is nonzero */
  while ((n = Rio_readnb(rio_dest, p, MAXLINE))) {
      Rio_writen(connfd, p, n);
      p += n;
  }
  return NULL;
}
/* Read the response HTTP headers from the upstream server 
 * returns NULL on success, or error message.
 */
const char *http_relay_resp_headers(int connfd, ReqData *req_d,
				    rio_t *rio_dest, int *size, char *type)
{
  char rec_buf[MAXLINE];
  int stat_code, l;
  char *pp;
  char resp_ver[MAXLINE], stat_mesg[MAXLINE];
  
  /* pass along response line */
  Rio_readlineb(rio_dest, rec_buf, MAXLINE);
  /* read response version to ensure compatibility */
  sscanf(rec_buf, "%s %d %s", resp_ver, &stat_code, stat_mesg);
  if (!strcmp(req_d->version, resp_ver))
    fprintf(stdout, "error, ver mismatch. client=%s, host=%s\n",
	    req_d->version, resp_ver);
  /* if i had code to downgrade the response to fit the client, 
  * here's where the function call would go. also to check and see
  * if it's an upgrade or a downgrade; don't know from strcmp. although
  * if you save the response value you could tell, but that assumes ascii.
  */
  Rio_writen(connfd, rec_buf, strlen(rec_buf));
  //  fprintf(stdout, "%s", rec_buf);
  
  /* extract size/type from headers and relay to client  */
  while(strcmp(rec_buf, "\r\n")) {
    Rio_readlineb(rio_dest, rec_buf, MAXLINE);

    /* TODO: should change this to log later */
    fprintf(stdout, "%s", rec_buf);

    /* save response info - type of file and filesize */
    if ( (strstr(rec_buf, "Content-Length") ||
		 strstr(rec_buf, "Content-length"))) {
      l = strlen("Content-length: ");
      pp = rec_buf + l;
      *size = atoi(pp);
    }
    else if (type && (strstr(rec_buf, "Content-Type:") ||
		      strstr(rec_buf, "Content-type:"))) {
      l = strlen("Content-type: ");
      pp = rec_buf + l;
      //int i = 0;
      //      while (*pp && (*pp != '\r')) {
      //	type[i] = *pp;
      //	i++;
      //      }
      strncpy(type, pp, MAXLINE);
      //      int i = strlen(type);
      //      type[i - 3] = '\0';
    }

    Rio_writen(connfd, rec_buf, strlen(rec_buf));
  }
  return NULL; 
}

void http_serve(int connfd, void *obj_buf, int size, char *type)
{
  char ret_buf1[MAXLINE];
  char ret_buf2[MAXLINE];
  sprintf(ret_buf1, "HTTP/1.0 200 OK \r\n");
  sprintf(ret_buf2, "%sServer: Brian's Web Proxy\r\n", ret_buf1);
  sprintf(ret_buf1, "%s%s", ret_buf2, connection_close);
  sprintf(ret_buf2, "%sContent-length: %d\r\n", ret_buf1, size);
  sprintf(ret_buf1, "%sContent-type: %s\r\n", ret_buf2, type); 
  Rio_writen(connfd, ret_buf1, strlen(ret_buf1));
  printf("%s", ret_buf1);
  Rio_writen(connfd, obj_buf, size);
}

/* Send an HTTP error message. */
void http_err(int fd, char *cause, char *errnum,
		 char *shortmsg, const char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF], body2[MAXBUF];
  
  /* build the HTTP response body */
  /* heads up - assumes everything in total < MAXBUFF */
  sprintf(body, "<html><title>Proxy Error</title>");
  sprintf(body2, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body2, errnum, shortmsg);
  sprintf(body2, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Bright Proxy</em>\r\n", body2);


  /* build & print the HTTP response headers*/
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));

  /* Print the response body */
  Rio_writen(fd, body, strlen(body));

  fprintf(stderr, "%s: %s\n", errnum, shortmsg);
  fprintf(stderr, "%s: %s\n", longmsg, cause);
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

/* Manually add carriage-return-line-feed to end of a string 
* and null out any extra stuff that was added in the overflow.
*/
static inline void add_crlfnull(char *buf, int max)
{
  int l;
  if ((l = strlen(buf)) >= max) {
    buf[max - 3] = '\r';
    buf[max - 2] = '\n';
    buf[max - 1] = '\0';
    for (; max <= l; l--) {
      buf[l] = '\0';
    }
  }
}

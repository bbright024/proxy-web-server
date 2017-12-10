static void segflt_handler(int sig);
static int check_request_errors(int connfd, char *method, char *uri, char *version);
static int parse_uri(char *url, char *file, char *host, char *port);
static void clienterror(int fd, char *cause, char *errnum,
		 char *shortmsg, char *longmsg);

/* 
 * Parse the uri to extract hostname, filename, and port.
 * Uses defaults localhost, /, and 80 if any are excluded.
 * Returns 0 on success or negative if an error occurs
 *   -INVALID_URL when an improper URL is passed
 */
static int parse_uri(char *url, char *file, char *host, char *port)
{
  char *url_pos = url;           	/* traverse the url */
  char *file_pos = file;		/* traverse filename array */
  char *port_pos = port;		/* traverse destination port array */
  char *host_pos = host;		/* traverse hostname array */
  char *pos;				/* for strstr search */

  if ((pos = strstr(url_pos, "http://"))) {
    pos += 7;
    url_pos = pos;
  }
  else
    pos = url_pos;

  /* check if the url contains a hostname */
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
    strcpy(port, "80");

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
    strcpy(file, "/");
  
  return 0;
}

/* if a segfault ever occurs, exit.. would like to find a way to 
 * close the faulting thread, without memory leaks.
 *
 * the info about the seg fault is stored in the ucontext
 * struct in <ucontext.h>

static void segflt_handler(int sig)
{
  sio_puts("in segflthandler\n");
  exit(-1);
  return;
}
 */
/* Checks the request line for errors, setting up the appropriate 
 * display page for the client to let them know what the problem was
 */
/* 
static int check_request_errors(int connfd, char *method, char *uri, char *version)
{
  if (strcasecmp(method, "GET")) {
    clienterror(connfd, method, "501", "Not implemented",
		"this proxy does not implement this method");
    return 0;
  }
  else if (strstr(uri, "https://")) {
    clienterror(connfd, method, "505", "Not supported",
		"this proxy does not implement https.");
    return 0;
  }
  else if (strcasecmp(version, "HTTP/1.1") &&
	   strcasecmp(version, "HTTP/1.0")) {
    clienterror(connfd, version, "505", "Not supported",
		"this proxy does not support this verson of HTTP");
    return 0;
  }
  return 1;
}
static inline void thread_uninit(void *obj_buf, int connfd)
{
  Close(connfd);
  free(obj_buf);
}
 */
/* builds and sends an error-response html page to the client  */
static void clienterror(int fd, char *cause, char *errnum,
		 char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* build the HTTP response body */
  sprintf(body, "<html><title>Proxy Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Bright Proxy</em>\r\n", body);

  /* build & print the HTTP response headers*/
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));

  /* Print the response body */
  Rio_writen(fd, body, strlen(body));
}




/* code graveyard: before integration */
    //  char method[MAXLINE], uri[MAXLINE], version[MAXLINE];	/* bufs for req line */
  //char host[MAXLINE], filename[MAXLINE], dest_port[MAXLINE]; /* bufs for url */


  /* Read request line & break it up*/
  //  Rio_readinitb(&rio_origin, connfd);
  //  Rio_readlineb(&rio_origin, buf, MAXLINE);
  //sscanf(buf, "%s %s %s", method, uri, version);

  //  if (check_request_errors(connfd, method, uri, version) == 0){
  //    thread_uninit(obj_buf, connfd);
  //    return NULL;
  //  }
    
  /* parse url to grab desired filename, host, port in 
   * seperate string buffers */
  //  if (parse_uri(uri, filename, host, dest_port)) {
  //    clienterror(connfd, uri, "400", "Bad request",
  //		"request could not be understood by the server");
  //  thread_uninit(obj_buf, connfd);
  //    return NULL;
  //  }

/* Brian's Proxy Web Server
 * 12/7/2017
 *
 * A Multithreaded, I/O multiplexing, caching web server
 *  -currently conforms to HTTP/1.0 
 *  -only implements GET
 *  -caches objects smaller than 102400 bytes
 *  -uses LRU algorithm to replace objects
 *
 * All rights to CSAPP and the folks at CMU for giving the framework,
 * tiny web server, testing code, and instructions to help me out with this.
 * 
*/

#include <sys/select.h>
#include <stdio.h>
#include "csapp.h"
#include "cache.h"
#include "http.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* User-defined port range */
#define PORT_ETH_MAX 65535
#define PORT_ETH_MIN 1025

/* Error codes */
#define INVALID_URL 1
#define E_NEXTHOP_CONN 3

/* Strings for response/request headers */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/100.3\r\n";
static const char *connection = "Connection: close\r\n";
static const char *proxy_con = "Proxy-Connection: close\r\n";

/* Global static variables */
static char *proxy_port;



/* Function definitions */
static void run_proxy();
static void *command(void *vargp);
static void *server_thread(void *vargp);
static void ret_cached_obj(void *obj_buf, int connfd, int size, char *type);
static int open_next_hop(int connfd, char *host, char *filename,
			 char *dest_port, void *obj_buf);
static void clienterror(int fd, char *cause, char *errnum,
		 char *shortmsg, char *longmsg);
static int parse_uri(char *url, char *file, char *host, char *port);
static void read_requesthdrs(rio_t *rp);
static void segflt_handler(int sig);
static int check_request_errors(int connfd, char *method, char *uri, char *version);
static inline void thread_uninit(void *obj_buf, int connfd);


/* Check args, set up signal handlers, init the cache, and start the proxy */
int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "format: %s <port num>\n", argv[0]);
    exit(0);
  }

  /* check if given port value in ephemeral range
   * if the strlen is > 5, obvious out of range & could be malicious
   */
  if (strlen(argv[1]) > 5) {
    fprintf(stderr, "port must be in the unpriv range!\n");
    exit(0);
  }
  int port;
  port = atoi(argv[1]);
  if (port < PORT_ETH_MIN || port > PORT_ETH_MAX) {
    fprintf(stderr, "port must be in the unpriv range!\n");
    exit(0);
  }

  proxy_port = argv[1];
  Signal(SIGSEGV, segflt_handler); 
  Signal(SIGPIPE, SIG_IGN); //ignores any sigpipe errors

  
  cache_init();
  run_proxy();
  
  // should never get here, but if it does, clean stuff
  cache_free_all();
  fprintf(stderr, "server terminated\n");
  return 0;
}

/* set up listening socket and call doit 
 * to service any requests.
 */
static void run_proxy()
{
  int listenfd, *connfdp;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t tid;
  char client_hostname[MAXLINE], client_port[MAXLINE];
  fd_set read_set,  ready_set;
  
  // open a listening socket to accept client requests
  listenfd = Open_listenfd(proxy_port);

  FD_ZERO(&read_set);
  FD_SET(STDIN_FILENO, &read_set);
  FD_SET(listenfd, &read_set);
  
  while (1) {
    ready_set = read_set;
    Select(listenfd+1, &ready_set, NULL, NULL, NULL);

    /* lets server respond to keyboard input, as well as client requests. */
    /* there is, however, a race to stdout if stdout is not changed. */
    if (FD_ISSET(STDIN_FILENO, &ready_set)){
      Pthread_create(&tid, NULL, command, NULL);
    }
    if (FD_ISSET(listenfd, &ready_set)){
      clientlen = sizeof(struct sockaddr_storage);
      //blocks in Accept until a connection request succeeds
      connfdp = Malloc(sizeof(int));
      *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
      Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
		  client_port, MAXLINE, 0);
      printf("Connected to (%s, %s)\n", client_hostname, client_port);
      Pthread_create(&tid, NULL, server_thread, connfdp);
    }
  }
}

static void *command(void *vargp)
{
  Pthread_detach(pthread_self());
  char buf[MAXLINE];
  if (!Fgets(buf, MAXLINE, stdin))
    return NULL;
  print_cache(1);
  return NULL;
}

/* 
 *  process the request, forward to the destination, and pass back the response.
 */
static void *server_thread(void *vargp)
{
  /* thread initialization */
  int connfd = *((int *)vargp);
  Pthread_detach(pthread_self());
  Free(vargp);

  /* variables for caching/retrieving objects */
  int is_cached = 0;
  void *obj_buf;
  char type[MAXLINE];
  size_t size;
  obj_buf = Malloc(MAX_OBJECT_SIZE);
  memset(obj_buf, 0, MAX_OBJECT_SIZE);
  
  rio_t rio_origin; 		/* rio for originating request */
  char buf[MAXLINE];		/* buf for originating request */
  char method[MAXLINE], uri[MAXLINE], version[MAXLINE];	/* bufs for req line */
  char host[MAXLINE], filename[MAXLINE], dest_port[MAXLINE]; /* bufs for url */

  /* Read request line & break it up*/
  Rio_readinitb(&rio_origin, connfd);
  Rio_readlineb(&rio_origin, buf, MAXLINE);
  sscanf(buf, "%s %s %s", method, uri, version);

  if (check_request_errors(connfd, method, uri, version) == 0){
    thread_uninit(obj_buf, connfd);
    return NULL;
  }
    
  /* parse url to grab desired filename, host, port in 
   * seperate string buffers */
  if (parse_uri(uri, filename, host, dest_port)) {
    clienterror(connfd, uri, "400", "Bad request",
		"request could not be understood by the server");
    thread_uninit(obj_buf, connfd);
    return NULL;
  }
  
  /* getting here means the request line is OK.
   * now check the request headers. */
  read_requesthdrs(&rio_origin);

  /* check host/filename for a match in the cache before opening a new conn */

  is_cached = check_cache(host, filename, obj_buf, type, &size);
  if (is_cached < 0) {
    printf("error");
    thread_uninit(obj_buf, connfd);
    return NULL;
  }
  else if (is_cached) {
    /* write the obj_buf to the connfd rio after  */
    ret_cached_obj(obj_buf, connfd, (int)size, type);
  }
  else if (!is_cached) {
    /* open a connection to the next hop server */
    int r;
    r = open_next_hop(connfd, host, filename, dest_port, obj_buf);
    if (r == -E_NEXTHOP_CONN) {
      clienterror(connfd, method, "400", "Bad request",
		"request could not be understood by the server");
      thread_uninit(obj_buf, connfd);
      return NULL;      
    }
  }

  thread_uninit(obj_buf, connfd);
  return NULL;
}

/* builds & sends the response to a client when the object is cached */
static void ret_cached_obj(void *obj_buf, int connfd, int size, char *type)
{
    char ret_buf[MAXLINE];
    sprintf(ret_buf, "HTTP/1.0 200 OK \r\n");
    sprintf(ret_buf, "%sServer: Brian's Web Proxy\r\n", ret_buf);
    sprintf(ret_buf, "%s%s", ret_buf, connection);
    sprintf(ret_buf, "%sContent-length: %d\r\n", ret_buf, (int)size);
    sprintf(ret_buf, "%sContent-type: %s\r\n", ret_buf, type); //heads up, a \r\n is saved in type
    Rio_writen(connfd, ret_buf, strlen(ret_buf));
    printf("%s", ret_buf);
    Rio_writen(connfd, obj_buf, size);
}

/* Opens a connection to the requested server to grab the object.
 * This is called if the requested object is not present in the cache.
 */
static int open_next_hop(int connfd, char *host, char *filename, char *dest_port, void *obj_buf)
{
  rio_t rio_destin;		/* rio for destination server */
  int destin_fd;		/* socket for destination server */
  char dest_buf[MAXLINE];	/* buffer for destination comm */
  
  if ((destin_fd = open_clientfd(host, dest_port)) < 0)
    return -E_NEXTHOP_CONN;
  
  Rio_readinitb(&rio_destin, destin_fd);
  
  /* craft the request line & headers & write to the server */
  sprintf(dest_buf, "GET %s HTTP/1.0\r\n", filename);
  sprintf(dest_buf, "%sHost: %s\r\n", dest_buf, host);
  sprintf(dest_buf, "%s%s", dest_buf, user_agent_hdr);
  sprintf(dest_buf, "%s%s", dest_buf, connection);
  sprintf(dest_buf, "%s%s", dest_buf, proxy_con);
  // might need to get the rest of the headers
  // that are currently inside rio_rec
  sprintf(dest_buf, "%s\r\n", dest_buf);
  Rio_writen(destin_fd, dest_buf, strlen(dest_buf));

  char rec_buf[MAXLINE];
  int length = 0;
  char type[MAXLINE];
  int l;
  char *pp;
  Rio_readlineb(&rio_destin, rec_buf, MAXLINE);
  Rio_writen(connfd, rec_buf, strlen(rec_buf));
  /* proxy puts response line/headers into rec_buf & writes to client */
  while(strcmp(rec_buf, "\r\n")) {
    Rio_readlineb(&rio_destin, rec_buf, MAXLINE);
    printf("%s", rec_buf);


    /* save response info - type of file and filesize */
    if (strstr(rec_buf, "Content-Length") || strstr(rec_buf, "Content-length")) {
      l = strlen("Content-length: ");
      pp = rec_buf + l;
      length = atoi(pp);
    }
    else if (strstr(rec_buf, "Content-Type:") || strstr(rec_buf, "Content-type:")) {
      l = strlen("Content-type: ");
      pp = rec_buf + l;
      strncpy(type, pp, MAXLINE);
    }
    Rio_writen(connfd, rec_buf, strlen(rec_buf));
  }
  int cacheable = 0;
  char *p = NULL;
  if (length < MAX_OBJECT_SIZE) {
    cacheable = 1;
    p = (char *)obj_buf;
  }
    
  char body[MAXLINE];
  int n;

  /* proxy puts the file into body while # of bytes read is nonzero */
  while ((n = Rio_readlineb(&rio_destin, body, MAXLINE))) {
    if (cacheable) {
      memcpy(p, body, n);
      p += n;
    }

    Rio_writen(connfd, body, n);
  }
  if (cacheable) {
    printf("adding to cache:\n obj = %p \nlength = %u \nhost = %s \nfile = %s \ntype = %s\n", obj_buf,
	   length, host, filename, type);
    int r = add_to_cache(obj_buf, (size_t)length, host, filename, type);
    if (r < 0)
      printf("error - was not cached");
  }
  
  printf("closing both connections, done!\n");
  Close(destin_fd);
  return 0;
}

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

/* read out the rest of the request headers from the client.
 * TODO: have this do something. maybe eliminate this method and 
 * incorporate it into the main flow.
 */
static void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];
  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
 
}

/* if a segfault ever occurs, exit.. would like to find a way to 
 * close the faulting thread, without memory leaks.
 *
 * the info about the seg fault is stored in the ucontext
 * struct in <ucontext.h>
 */
static void segflt_handler(int sig)
{
  sio_puts("in segflthandler\n");
  exit(-1);
  return;
}

/* Checks the request line for errors, setting up the appropriate 
 * display page for the client to let them know what the problem was
 */
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

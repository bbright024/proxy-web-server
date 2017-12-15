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

/* TODO:  
 *  - fix the dangling \r\n in type in http
 *  - add support for POST
 *  - fix the size_t size thing for objects
 *  - write test code to find the free bug
 *  - change the linked list to a hash bucket chain
 */
#include <sys/select.h>
#include <stdio.h>
#include <includes/csapp.h>
#include <includes/cache.h>
#include <includes/http.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* User-defined port range */
#define PORT_ETH_MAX 65535
#define PORT_ETH_MIN 1025

/* Error codes */
#define INVALID_URL 1
#define E_NEXTHOP_CONN 3

/* Global static variables */
static char *proxy_port;
static int run = 1; 		/* while true, server keeps looping */

/* Function definitions */
static void run_proxy();
static void *command(void *vargp);
static void *process_client(void *vargp);
static int open_next_hop(int connfd, ReqData *req_d, rio_t *rio);
static inline void thread_uninit(void *obj_buf, int connfd, ReqData *req_d);
static inline int client_init(int connfd, ReqData **req_d, void **obj_buf);

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

  Signal(SIGPIPE, SIG_IGN); //ignores any sigpipe errors
  
  if (cache_init() < 0) {
    fprintf(stderr, "out of men");
    exit(0);
  }
    
  run_proxy();
  
  // should never get here, but if it does, clean stuff
  cache_free_all();
  fprintf(stderr, "server terminated\n");
   pthread_exit(0);
   // return 0;
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

  while (run) {
    ready_set = read_set;
    if(Select(listenfd+1, &ready_set, NULL, NULL, NULL) < 0)
      continue;

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
      Pthread_create(&tid, NULL, process_client, connfdp);
    }
  }
}

static void *command(void *vargp)
{
  Pthread_detach(pthread_self());
  free(vargp);
  char buf[MAXLINE];
  if (!Fgets(buf, MAXLINE, stdin)) {
    return NULL;
  }
  else if (buf[0] != 'p') {
    run = 0;
  }    
  else 
    print_cache(0);
  return NULL;
}

/*  Main thread for a client request.
 *  process the client's request - forward to the destination if not cached
 */
static void *process_client(void *vargp)
{
  /* thread initialization */
  int connfd = *((int *)vargp);
  Pthread_detach(pthread_self());
  free(vargp);

  /* allocate space for the metadata and the object itself.  */
  /* obj space is allocated here because faster & more secure to grab obj
   * immediately in cache search if found, and quits immediately if no mem. */
  ReqData *req_d;
  void *obj_buf;		
  if ((client_init(connfd, &req_d, &obj_buf)) < 0)
    return NULL;

  /* variables for caching/retrieving objects */
  char type[MAXLINE];
  size_t size = 0;
  int cacheable = 0;
  rio_t rio_origin; 	
  const char *errmsg;
  Rio_readinitb(&rio_origin, connfd);

  if ((errmsg = http_read_request_line(&rio_origin, req_d))){
    http_err(connfd, "invalid request", "403", "Forbidden", errmsg);
    thread_uninit(obj_buf, connfd, req_d);
    return NULL;
  }
  
  /* TODO: change flow here based on req_d->method & version
   *  i.e. a POST request cannot be cached   //  read_requesthdrs(&rio_origin);   */
  
  /* check cache for host/filename, get type/size if there  */
  if ((check_cache(req_d->host, req_d->filename, obj_buf, type, &size))) {
    http_serve(connfd, obj_buf, (int)size, type);
    thread_uninit(obj_buf, connfd, req_d);
    return NULL;
  }
  /* not in cache; open a connection to the next hop server */
  int destfd;
  rio_t rio_dest;
  if ((destfd = open_next_hop(connfd, req_d, &rio_dest)) < 0) {
    thread_uninit(obj_buf, connfd, req_d);
    return NULL;
  }

  http_relay_resp_headers(connfd, req_d, &rio_dest, (int *)&size, type);
  if (size  && size < MAX_OBJECT_SIZE) {
    fprintf(stdout, "cacheable set, size is %d\n", (int)size);
    cacheable = 1;
  }
  else if (size){
    /* make sure obj buff will be able to hold the object, even if not cacheable*/
    free(obj_buf);
    if ((obj_buf = malloc(size)) == NULL) {
      http_err(connfd, "memory", "500", "Proxy internal error", "proxy is out of memory");
      free(req_d);
      close(connfd);
      close(destfd);
      return NULL;
    }
  }
  
  http_relay_resp_body(connfd, &rio_dest, size, obj_buf);

  if (cacheable) {
    fprintf(stdout,
	    "adding to cache:\nobj = %p\nlength = %d\nhost = %s\nfile = %s\ntype = %s\n",
	    obj_buf, (int)size, req_d->host, req_d->filename, type);
    int r = add_to_cache(obj_buf, size, req_d->host, req_d->filename, type);
    if (r < 0)
      printf("error - was not cached");
  }
  close(destfd);
  close(connfd);
  free(req_d);
  free(obj_buf);
  return NULL;
}

/* Opens a connection to the requested server to grab the object.
 * Called if object is not present in the cache. Save obj if small enough.
 */
static int open_next_hop(int connfd, ReqData *req_d, rio_t *rio)
{
  const char *errmsg;
  int destfd;
  if ((destfd = open_clientfd(req_d->host, req_d->dest_port)) < 0) {
    http_err(connfd, req_d->host, "500", "Upstream error", "Proxy couldn't connect to host");
    return -1;
  }
  if ((errmsg = http_write_request(destfd, req_d))) {
    http_err(connfd, "bad request", "400", "Bad request", errmsg);
    return -1;
  }

  Rio_readinitb(rio, destfd);
  
  return destfd;
}

static inline void thread_uninit(void *obj_buf, int connfd, ReqData *req_d)
{
  Close(connfd);
  free(obj_buf);
  free(req_d);
}

/* allocate objects used by proxy threads */
static inline int client_init(int connfd, ReqData **req_d, void **obj_buf)
{
  
  if ((*obj_buf = malloc(MAX_OBJECT_SIZE)) == NULL) {
    http_err(connfd, "memory", "500", "Proxy internal error", "proxy is out of memory");
    close(connfd);
    return -1;
  }

  if ((*req_d = malloc(sizeof(ReqData))) == NULL){
    http_err(connfd, "memory", "500", "Proxy internal error", "proxy is out of memory");
    close(connfd);
    free(*obj_buf);
    return -1;
  }
  memset(*obj_buf, 0, MAX_OBJECT_SIZE);
  memset(*req_d, 0, sizeof(ReqData));
  return 0;
}

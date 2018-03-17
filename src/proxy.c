/* Brian's Proxy Web Server
 * 2017 - 2018
 *
 * A Multithreaded, I/O multiplexing, caching web server
 *
 * All rights to CSAPP and the folks at CMU for giving the framework,
 * tiny web server, testing code, and instructions to help me out with this.
 * 
*/

//#include <sys/select.h>
#include <stdio.h>
#include <includes/csapp.h>
#include <includes/cache.h>
#include <includes/http.h>
#include <includes/proxy.h>
#include <pthread.h>
#include <includes/sys_wraps.h>

/* Global static variables */
/* run: while true, server keeps looping. changed via user input. */
static int run = 1; 		

/* static function definitions */

static void *command(void *vargp);
static void *process_client(void *vargp);
static int open_next_hop(int connfd, ReqData *req_d, rio_t *rio);
static inline void thread_uninit(void *obj_buf, int connfd, ReqData *req_d);
static inline int client_init(int connfd, ReqData **req_d, void **obj_buf);

/* set up listening socket and call doit 
 * to service any requests.
 */
void run_proxy(char *proxy_port)
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
      //currently prints stuff once and exits.  needed a quick way to
      //exit the server after printing stuff to test mem leaks.
      //currently it causes issues for the driver testing script tho
    //      break;
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
  
  // should never get here, but if it does, clean stuff
  cache_free_all();

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
      return NULL;
  }
  
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

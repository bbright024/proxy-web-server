#include <stdio.h>
#include <includes/csapp.h>
#include <includes/proxy.h>
#include <includes/errors.h>
#include <includes/cache.h>

#define PORT_ETH_MAX 65535
#define PORT_ETH_MIN 1025

/* Check args, set up signal handlers, init the cache, and start the proxy */
int main(int argc, char *argv[])
{
  // if the strlen is > 5, obvious out of range & could be malicious
  if (argc != 2 || strlen(argv[1]) > 5)
    Usage(argv[0]);

  int port;
  port = atoi(argv[1]);
  if (port < PORT_ETH_MIN || port > PORT_ETH_MAX)
    Usage(argv[0]);

  Signal(SIGPIPE, SIG_IGN); //ignores any sigpipe errors
  
  if (cache_init() < 0)
    init_error("cache initialization error");
  
  run_proxy(argv[1]);

  fprintf(stdout, "server terminated\n");

  return 0;
}

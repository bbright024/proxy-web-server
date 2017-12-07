#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "csapp.h"

char *tiny_path = "./tiny/tiny";
char *proxy_path = "/home/tychocel/csapp/proxylab-handout/proxy";

int server_init(char *port_proxy, char *port_tiny);

int main(int argc, char *argv[]) {
  char *port_proxy;
  char *port_tiny;
  char *def_tiny = "8000";
  char *def_proxy = "8080";

  printf("argv[0] : %s\n", argv[0]);
  switch (argc) {
  case 3:
    port_proxy = argv[1];
    port_tiny = argv[2];
    break;
  case 2:
    port_proxy = argv[1];
    port_tiny = def_proxy;
    break;
  case 1:
    port_proxy = def_proxy;
    port_tiny = def_tiny;
    break;
  default:
    fprintf(stderr, "usage: test_proxy <proxy_port> <tiny_port>\n");
    fprintf(stderr, "or with no args, tiny port = 8000, proxy = 8000\n"); 
    exit(0);
  }

  if (server_init(port_proxy, port_tiny) < 0) {
    fprintf(stderr, "error starting procs\n");
    exit(0);
  }
  return 0;
}

int server_init(char *port_proxy, char *port_tiny)
{
  pid_t proxy;
  pid_t tiny;
  char *argv[3];
  argv[2] = NULL;
  if ((proxy = fork()) == 0) {
    argv[0] = proxy_path;
    argv[1] = port_proxy;
    printf("bout to exec proxy, %s\n", argv[0]);
    Execve(argv[0], argv, environ);
  }
  if (proxy < 0)
    return -1;
  
  if ((tiny = fork()) == 0) {
    argv[0] = tiny_path;
    argv[1] = port_tiny;
    printf("bout to exec tiny, %s\n", argv[0]);
    Execve(argv[0], argv, environ);
  } 
  if (tiny < 0)
    return -1;
  
  return 0;
}

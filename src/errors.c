#include <includes/csapp.h>
#include <includes/dbg.h>
#include <includes/errors.h>
void Usage(const char *prog)
{
  fprintf(stderr, "format: %s <port num: 1024 - 65535> \n ", prog);
  exit(0);
}

void init_error(const char *msg)
{
  unix_error(msg);
  exit(0);
}

void thread_unix_error(const char *msg)
{
  unix_error(msg);
  pthread_exit(NULL);
}

void unix_error(const char *msg) /* Unix-style error */
{
  //  log_err("error %s", msg );
  //  goto error;
  // error:
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  return;
  //    
    //    exit(0);
}


void posix_error(int code, char *msg) /* Posix-style error */
{
  //    log_err(msg);
        fprintf(stderr, "%s: %s\n", msg, strerror(code));
    //    exit(0);
}

void gai_error(int code, char *msg) /* Getaddrinfo-style error */
{
  //log_err(msg);
        fprintf(stderr, "%s: %s\n", msg, gai_strerror(code));
    //    exit(0);
}

void app_error(char *msg) /* Application error */
{
  //log_err(msg);
    fprintf(stderr, "%s\n", msg);
    //    exit(0);
}


void dns_error(char *msg) /* Obsolete gethostbyname error */
{
  //  log_err(msg);
 fprintf(stderr, "%s\n", msg);
    //    exit(0);
}

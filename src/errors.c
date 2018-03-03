#include <includes/csapp.h>
#include <includes/dbg.h>

void unix_error(char *msg) /* Unix-style error */
{
  log_err("error %s", msg );
  goto error;
 error:
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

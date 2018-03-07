#pragma once

/* Error codes */
#define INVALID_URL 1
#define E_NEXTHOP_CONN 3

void run_proxy(char *proxy_port);

/* part of what's needed to implement POST and other stuff,
 * also will need function pointer array
 */

void clienterror(int fd, char *cause, char *errnum,
		 char *shortmsg, char *longmsg);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
int parse_host(char *host);
void get(int fd, char *uri, char *version);

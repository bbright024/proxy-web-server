#pragma once

#define E_NO_SPACE 2

void Usage(const char *prog);
void init_error(const char *msg);
void unix_error(const char *msg);
void posix_error(int code, char *msg);
void dns_error(char *msg);
void gai_error(int code, char *msg);
void app_error(char *msg);
void thread_unix_error(const char *msg);

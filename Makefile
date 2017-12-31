# Makefile for Brian's Proxy
#
AR = ar
CC = gcc

# flags for cc/ld/etc.
CFLAGS += -g -Wall -I. -I./includes/ -O0 -fprofile-arcs -ftest-coverage -pg
LDFLAGS += -lpthread -L. 
ARFLAGS = rcs

# define common dependencies
OBJS = proxy.o cache.o http.o LinkedList.o csapp.o

all: clean proxy

coverage: proxy
	./proxy 8000
	gprof proxy > proxyoutput.stats
	gcov LinkedList.c
	gcov proxy.c
	gcov cache.c
	gcov http.c
	@echo "Look at .gcov files for coverage data"
	lcov -c -d . -o proxyinfo.info
	genhtml proxyinfo.info -o cov_html/

proxy: $(OBJS)
	$(CC) $(CFLAGS) -o proxy proxy.o csapp.o cache.o http.o LinkedList.o $(LDFLAGS)

%.o: %.c 
	$(CC) $(CFLAGS) -c $<

#csapp.o: csapp.c csapp.h
#	$(CC) -g -Wall -O0 -c csapp.c 


#test_proxy: test_proxy.o csapp.o
#	$(CC) $(CFLAGS) test_proxy.o csapp.o -o test_proxy $(LDFLAGS)

#the Checker
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' 		*.* || true

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz *.gcda *.gcno *.info gmon.out
	rm -Rf ./cov_html

FORCE:







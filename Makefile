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
HEADERS = #LinkedList.h http.h csapp.h cache.h

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

proxy: $(OBJS) $(HEADERS) FORCE
	$(CC) $(CFLAGS) -o proxy proxy.o csapp.o cache.o http.o LinkedList.o $(LDFLAGS)

%.o: %.c $(HEADERS) FORCE
	$(CC) $(CFLAGS) -c $<

#csapp.o: csapp.c csapp.h
#	$(CC) -g -Wall -O0 -c csapp.c 


#test_proxy: test_proxy.o csapp.o
#	$(CC) $(CFLAGS) test_proxy.o csapp.o -o test_proxy $(LDFLAGS)

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz *.gcda *.gcno *.info gmon.out
	rm -Rf ./cov_html

FORCE:


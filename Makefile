
# Makefile for Brian's Proxy
#

CC = gcc

# flags for cc/ld/etc.
CFLAGS += -g -Wall -I. -I.. -O0 -fprofile-arcs -ftest-coverage -pg
LDFLAGS += -lpthread -L. #-fprofile-arcs -ftest-coverage

all: proxy 
	./proxy 8000
	gprof proxy > proxyoutput.stats
	gcov LinkedList.c
	gcov proxy.c
	gcov cache.c
	gcov http.c
	@echo "Look at .gcov files for coverage data"
	lcov -c -d . -o proxyinfo.info
	genhtml proxyinfo.info -o cov_html/

proxy: proxy.o csapp.o cache.o LinkedList.o http.o csapp.h cache.h LinkedList.h http.h
	$(CC) $(CFLAGS) -o proxy proxy.o csapp.o cache.o http.o LinkedList.o $(LDFLAGS)

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c 

proxy.o: proxy.c csapp.h cache.h
	$(CC) $(CFLAGS) -c proxy.c $(LDFLAGS)

cache.o: cache.c cache.h csapp.h
	$(CC) $(CFLAGS) -c cache.c $(LDFLAGS)

test_proxy.o: test_proxy.c csapp.h
	$(CC) $(CFLAGS) -c test_proxy.c

LinkedList.o: LinkedList.c LinkedList.h
	$(CC) $(CFLAGS) -c LinkedList.c $(LDFLAGS)

http.o: http.c http.h
	$(CC) $(CFLAGS) -c http.c $(LDFLAGS)

#proxy: proxy.o csapp.o cache.o LinkedList.o http.o
#	$(CC) $(CFLAGS) proxy.o csapp.o cache.o http.o LinkedList.o -o proxy $(LDFLAGS)


test_proxy: test_proxy.o csapp.o
	$(CC) $(CFLAGS) test_proxy.o csapp.o -o test_proxy $(LDFLAGS)

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz


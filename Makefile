
# Makefile for Proxy Lab 
#

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: proxy test_proxy

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c csapp.h cache.h
	$(CC) $(CFLAGS) -c proxy.c

cache.o: cache.c cache.h csapp.h
	$(CC) $(CFLAGS) -c cache.c

test_proxy.o: test_proxy.c csapp.h
	$(CC) $(CFLAGS) -c test_proxy.c

LinkedList.o: LinkedList.c LinkedList.h
	$(CC) $(CFLAGS) -c LinkedList.c

http.o: http.c http.h
	$(CC) $(CFLAGS) -c http.c

proxy: proxy.o csapp.o cache.o LinkedList.o http.o
	$(CC) $(CFLAGS) proxy.o csapp.o cache.o http.o LinkedList.o -o proxy $(LDFLAGS)

test_proxy: test_proxy.o csapp.o
	$(CC) $(CFLAGS) test_proxy.o csapp.o -o test_proxy $(LDFLAGS)

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz


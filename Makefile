# Makefile for Brian's Proxy
#
AR = ar
CC = gcc

# flags for cc/ld/etc.
CFLAGS += -g -Wall  -I. -I./src/ -O0  -fprofile-arcs -ftest-coverage -pg
LDFLAGS += -lpthread -L. 
ARFLAGS = rcs

# define common dependencies
C_SOURCES=$(wildcard src/**/*.c src/*.c)
OBJS=$(patsubst %.c,%.o,$(C_SOURCES))
#problem with including all header files is it grabs private ones too
#leads to make expecting a .o file for the priv headers 
H_SOURCES=$(wildcard src/includes/*.h)
AUX=$(patsubst %.h,%,$(H_SOURCES))


TARGET=./bin/proxy
# old
#OBJS = proxy.o cache.o http.o LinkedList.o csapp.o

all: $(TARGET)

coverage: #CFLAGS += -fprofile-arcs -ftest-coverage -pg
coverage: all
	./bin/proxy 8000
	gprof ./bin/proxy > ./build/proxyoutput.stats
	gcov ./src/LinkedList.c
	gcov ./src/proxy.c
	gcov ./src/cache.c
	gcov ./src/http.c
	@echo "Look at .gcov files for coverage data"
	lcov -c -d ./src/ -o ./build/proxyinfo.info
	genhtml ./build/proxyinfo.info -o cov_html/

$(TARGET): build $(OBJS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c $(AUX)
	$(CC) $(CFLAGS) -c $<

build:
	@mkdir -p build
	@mkdir -p bin

#csapp.o: csapp.c csapp.h
#	$(CC) -g -Wall -O0 -c csapp.c 


#test_proxy: test_proxy.o csapp.o
#	$(CC) $(CFLAGS) test_proxy.o csapp.o -o test_proxy $(LDFLAGS)

#the Checker
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' 		*.* || true

clean:
	rm -f ./bin/*
	rm -f src/*.o src/*.gcda src/*.gcno
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz *.gcda *.gcno *.info gmon.out
	rm -f ./*.stats
	rm -Rf ./cov_html

FORCE:







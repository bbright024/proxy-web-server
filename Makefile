# Makefile for Brian's Proxy
#
AR = ar
CC = gcc

# flags for cc/ld/etc.
CFLAGS += -g -Wall -I./src/ -O2  
LDFLAGS += -lpthread -L./build/
ARFLAGS = rcs

# define common dependencies
C_SOURCES=$(wildcard src/**/*.c src/*.c)
OBJS=$(patsubst %.c,%.o,$(C_SOURCES))
H_SOURCES=$(wildcard src/includes/*.h)
AUX=$(patsubst %.h,%,$(H_SOURCES))


TARGET=./bin/proxy

LIBTARGET=./build/libcsapp.a
LIBSRC = ./lib/csapp.c
LIBOBJS = ./lib/csapp.o
SO_TARGET=$(patsubst %a,%.so,$(LIBTARGET))

# old
#OBJS = proxy.o cache.o http.o LinkedList.o csapp.o

all: $(LIBTARGET) $(TARGET) $(SO_TARGET)

#either do gcov or lcov, not both - some weird bugs.
coverage: CFLAGS= -g -Wall -I./src/ -O0 -fprofile-arcs -ftest-coverage -pg
coverage: clean all
	./bin/proxy 8000
	gprof -b ./bin/proxy ./gmon.out > ./build/proxyoutput.stats
#	gcov ./src/LinkedList.c
#	gcov ./src/proxy.c
#	gcov ./src/cache.c
#	gcov ./src/http.c
	@echo "Look at .gcov files for coverage data"
	lcov -b /home/tychocel/proxy  -c -d ./src/ -o ./build/proxyinfo.info
	genhtml ./build/proxyinfo.info -o ./build/cov_html/
	mv ./gmon.out ./build/

$(TARGET): build $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBTARGET)

$(LIBTARGET): CFLAGS += -fPIC
$(LIBTARGET): build $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	ranlib $@

$(SO_TARGET): $(LIBTARGET) $(LIBOJBS)
	$(CC) -shared -o $@ $(LIBOBJS)

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
#the || true sets make to not exit if results are found
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' $(C_SOURCES) || true

clean:
	rm -f $(TARGET)
	rm -f src/*.o src/*.gcda src/*.gcno
	rm -f *~ *.o proxy core *.tar *.zip
	rm -f *.gzip *.bzip *.gz *.gcda *.gcno *.info gmon.out
	rm -f ./*.stats
	rm -Rf ./build
	rm -f ./build/*.a

FORCE:







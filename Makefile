# Makefile for Brian's Proxy
#
AR = ar
CC = gcc

# flags for cc/ld/etc.
CFLAGS += -g -Wall -I./src/ -O2  
LDFLAGS += -lpthread #-L./build/ -ldl 
ARFLAGS = rcs

# define common dependencies
C_SOURCES=$(wildcard src/**/*.c src/*.c)
OBJS=$(patsubst %.c,%.o,$(C_SOURCES))

H_SOURCES=$(wildcard src/includes/*.h)
AUX=$(patsubst %.h,%,$(H_SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

T_AUX_SRC=$(wildcard tests/*.h)
T_AUX=$(patsubst %.h,%,$(T_AUX_SRC))

TARGET=./bin/proxy

LIBTARGET=./build/libcsapp.a
LIBSRC = ./lib/csapp.c
LIBOBJS = ./lib/csapp.o

# turn on for dynamic linking
#SO_TARGET=$(patsubst %.a,%.so,$(LIBTARGET))

all: $(LIBTARGET) $(TARGET)
# turn on for dynamic linking
#all:  $(SO_TARGET)

#either do gcov or lcov, not both - some weird bugs.
coverage: CFLAGS = -g -Wall -I./src/ -O0 -fprofile-arcs -ftest-coverage -pg
coverage:  all
	./bin/proxy 8000
	lcov -b /home/tychocel/proxy  -c -d ./src/  -o ./build/proxyinfo.info
	genhtml ./build/proxyinfo.info -o ./build/cov_html/
	@echo "Open ./build/cov_html files in a browser for coverage data"

$(TARGET): build $(OBJS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBTARGET)

$(LIBTARGET): CFLAGS += -fPIC
$(LIBTARGET): build $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	ranlib $@

# turn on for dynamic linking 
#$(SO_TARGET): $(LIBTARGET) $(LIBOJBS)
#	$(CC) -shared -o $@ $(LIBOBJS)

%.o: %.c $(AUX)
	$(CC) $(CFLAGS) -c $<

build:
	@mkdir -p build
	@mkdir -p bin

$(TESTS): 
	$(CC) $(CFLAGS) $@.c $(LDFLAGS) -o $@

.PHONY: tests
tests: CFLAGS = -g -Wall -DNDEBUG -I./src/ -O0 
tests: all $(TESTS) $(T_AUX)
	bash ./tests/runtests.sh || true

#the Checker
#the || true sets make to not exit if results are found
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' $(C_SOURCES) || true

clean:
	rm -Rf ./bin/
	rm -Rf ./build/
	rm -f ./*/*.o ./*/*.gcda ./*/*.gcno
	rm -f *~ *.o proxy core *.tar *.zip
	rm -f *.gzip *.bzip *.gz *.gcda *.gcno *.info gmon.out
	rm -f ./*.stats

FORCE:

#old code - for having coverage data in the tests. doesn't work when
#output is redirected to a tests.log so I removed it.
# -fprofile-arcs -ftest-coverage -pg
#	mv *.gcda ./tests/
#	lcov -b /home/tychocel/proxy  -c -d ./tests/ -o ./build/testsinfo.info
#	genhtml ./build/proxyinfo.info -o ./build/tests_cov__html/
#	mv ./gmon.out ./build/
#	$(CC) $(CFLAGS) $(TEST_SRC) $(LDFLAGS) -o ./tests/debug_tests 
#	./tests/debug_tests ../README
#csapp.o: csapp.c csapp.h
#	$(CC) -g -Wall -O0 -c csapp.c 


#test_proxy: test_proxy.o csapp.o
#	$(CC) $(CFLAGS) test_proxy.o csapp.o -o test_proxy $(LDFLAGS)

# program doesn't do enough work for gprof to be useful.
#	gprof -b ./bin/proxy ./gmon.out > ./build/proxyoput.stats
#	mv ./gmon.out ./build/

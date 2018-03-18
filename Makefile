######################################
# Makefile for Brian's Proxy 2018
######################################

#####################
# flags for cc/ld/etc.
#####################
AR = ar
CC = gcc

CFLAGS += -g -Wall -I./src/ -O0 -rdynamic $(OPTFLAGS)
COVFLAGS = -fprofile-arcs -ftest-coverage -g -pg
LDFLAGS += -lpthread -ldl $(OPTLIBS) #-L./build/ 
ARFLAGS = rcs
PREFIX ?= /usr/local

###########################
# define common dependencies
###########################

C_SOURCES=$(wildcard src/**/*.c src/*.c)
OBJS=$(patsubst %.c,%.o,$(C_SOURCES))

H_SOURCES=$(wildcard src/includes/*.h)
AUX=$(patsubst %.h,%,$(H_SOURCES))

TEST_SRC=$(wildcard tests/*_test.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TESTABLE_OBJS = ./src/cache.o ./src/http.o ./src/LinkedList.o ./src/HashTable.o ./src/errors.o ./src/pthread_wraps.o ./src/network_wraps.o ./src/syscall_wraps.o ./src/proxy.o ./src/posix_wraps.o
TARGET=./bin/proxy


LIBTARGET=./build/libproxy.a
SO_TARGET=$(patsubst %.a,%.so,$(LIBTARGET))

# in case library source code in seperate directory
LIBSRC=$(wildcard lib/*.c)
LIBOBJS=$(patsubst %.c, %.o, $(LIBSRC))

# directory variables
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

# if LinkedList & HashTable need to be tested:
#DS_OBJS = LinkedList.o HashTable.o

##################
# the build rules
##################

all: build $(LIBTARGET) $(TARGET)
# turn on for dynamic linking
#all:  $(SO_TARGET)

# coverage will run the proxy - the user has to end it, preferably
# after using the proxy with a browser to test coverage.
coverage: CFLAGS = -Wall -I./src/ -O0 $(COVFLAGS)
coverage:  all
	./bin/proxy 8000
	lcov -b /home/bbright/proxy  -c -d ./src/  -o ./build/proxyinfo.info
	genhtml ./build/proxyinfo.info -o ./build/proxy_cov_html/
	@echo "Open ./build/cov_html files in a browser for coverage data"

test_coverage: clean build $(LIBTARGET) 
test_coverage: CFLAGS = -Wall -I./src/ -O0 $(COVFLAGS) 
test_coverage: $(OBJS) $(TESTS)

$(TESTS):
	$(CC) $(CFLAGS) $@.c $(LIBTARGET) -o ./bin/$@ $(TESTABLE_OBJS) $(LDFLAGS) $(LIBTARGET)
	./bin/$@
	lcov -b $(mkfile_path) -c -d ./ -o ./$@info.info
	genhtml ./$@info.info -o ./$@_html/
	@echo "Open html files in a browser for coverage data"
#todo - have it so lcov only grabs the gcda of the files it's testing - presently grabs results from every .c file
# could do it with pattern matching each test, but might be tricky.
# at the moment it all works fine, just kind of misleading to see 22% coverage at top layer.

# in case gprof is useful - timing data for the program.
#	gprof -b ./bin/proxy ./gmon.out > ./build/proxyoput.stats
#	mv ./gmon.out ./build/

$(TARGET): build $(OBJS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBTARGET)

$(LIBTARGET): CFLAGS = -g -Wall -I./src/ -O0 -fPIC
$(LIBTARGET): build $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	ranlib $@

%.o: %.c $(AUX)
	$(CC) $(CFLAGS) -c $< 

.PHONY: build
build:  
	@mkdir -p ./build
	@mkdir -p ./bin/tests
	$(MAKE) TAGS

.PHONY: tests
tests: CFLAGS += $(TARGET)
tests: $(TESTS) 


.PHONY: TAGS tags
TAGS tags:
	find ./src/ -type f -name "*.[ch]" | xargs etags -

# for a list of file dependencies
.PHONY: dep depend
dep depend:
	$(CC) $(CFLAGS) -MM ./src/*.c 
#the Checker
#the || true sets make to not exit if results are found
.PHONY: check
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' $(C_SOURCES) || true

# manually remove TAGS if you really need to
.PHONY: clean
clean:
	rm -Rf ./bin/*
	rm -Rf ./build/*
	rm -rf ./*/*_html/
	rm -rf 'find . -name "*.dSYM" -print'
	find . -type f \( -name "TAGS" -o -name "*.o" -o -name "*.gc*" -o -name ".stats" \
-o -name "*.log" -o -name "*.info" -o -name "gmon.out" -o -name "*_tests" \) -exec rm {} \;
	$(MAKE) TAGS


FORCE:

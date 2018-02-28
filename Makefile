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

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=./bin/proxy


LIBTARGET=./build/libproxy.a
SO_TARGET=$(patsubst %.a,%.so,$(LIBTARGET))

# in case library source code in seperate directory
LIBSRC=$(wildcard lib/*.c)
LIBOBJS=$(patsubst %.c, %.o, $(LIBSRC))

# directory variables
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

##################
# the build rules
##################

all: clean $(LIBTARGET) $(TARGET)
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
# in case gprof is useful - timing data for the program.
#	gprof -b ./bin/proxy ./gmon.out > ./build/proxyoput.stats
#	mv ./gmon.out ./build/

$(TARGET): build $(OBJS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBTARGET)

$(LIBTARGET): CFLAGS += -fPIC
$(LIBTARGET): build $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	ranlib $@

%.o: %.c $(AUX)
	$(CC) $(CFLAGS) -c $<

build:
	@mkdir -p build
	@mkdir -p bin

.PHONY: tests
tests: CFLAGS += $(TARGET)
tests: $(TESTS) 
	bash ./tests/runtests.sh || true

#the Checker
#the || true sets make to not exit if results are found
.PHONY: check
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' $(C_SOURCES) || true

.PHONY: clean
clean:
	rm -rf build $(OBJS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf 'find . -name "*.dSYM" -print'
	rm -Rf ./bin/
	rm -Rf ./build/
	rm -f ./*/*.o ./*/*.gcda ./*/*.gcno
	rm -f *~ *.o proxy core *.tar *.zip
	rm -f *.gzip *.bzip *.gz *.gcda *.gcno *.info gmon.out
	rm -f ./*.stats
	rm -f ./tests/*_tests
FORCE:

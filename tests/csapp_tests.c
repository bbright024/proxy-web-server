#include "minunit.h"
#include <dlfcn.h>

typedef int (*lib_function)(const char *data);
char *lib_file = "build/libcsapp.so";
void *lib = NULL;

int check_function(const char *func_to_run, const char *data,
		    int expected)
{
  lib_function func = dlsym(lib, func_to_run);
  check(func != NULL,
	"Did not find %s funciton in the library %s: %s", func_to_run,
	lib_file, dlerror());

  int rc = func(data);
  check(rc == 0, "Function %s return %d for data: %s", func_to_run,
	rc, data);
  return 1;
 error:
  return 0;
}
char *test_dlopen()
{
  lib = dlopen(lib_file, RTLD_NOW);
  mu_assert(lib != NULL, "Failed to open library to test");
  
  return NULL;
}
char *test_functions()
{
  mu_assert(check_function("testtest", "hello", 0),
	    "testtest failed");
  //  mu_assert(check_function("uppercase", "Hello", 0),
  //	    "uppercase failed");
  //mu_assert(check_function("lowercase", "Hello", 0),
  //	    "lowercase failed");
  
  return NULL;
}
char *test_failures()
{
  //  mu_assert(check_function("fail_on_purpose", "Hello", 1),
  //	    "fail_on_purpose should fail");

  return NULL;
}
char *test_dlclose()
{
  int rc = dlclose(lib);
  mu_assert(rc == 0, "failed to close lib.");
  
  return NULL;
}
char *all_tests()
{
  mu_suite_start();

  mu_run_test(test_dlopen);
  mu_run_test(test_functions);
  mu_run_test(test_failures);
  mu_run_test(test_dlclose);
  
  return NULL;
}

RUN_TESTS(all_tests);

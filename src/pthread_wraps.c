#include <includes/errors.h>
#include <pthread.h>
#include <includes/sys_wraps.h>
/************************************************
 * Wrappers for Pthreads thread control functions
 ************************************************/

void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp, 
		    void * (*routine)(void *), void *argp) 
{
    int rc;
    if ((rc = pthread_create(tidp, attrp, routine, argp)) != 0)
	posix_error(rc, "Pthread_create error");
}

void Pthread_cancel(pthread_t tid) {
    int rc;

    if ((rc = pthread_cancel(tid)) != 0)
	posix_error(rc, "Pthread_cancel error");
}

void Pthread_join(pthread_t tid, void **thread_return) {
    int rc;

    if ((rc = pthread_join(tid, thread_return)) != 0)
	posix_error(rc, "Pthread_join error");
}

void Pthread_detach(pthread_t tid) {
    int rc;
    if ((rc = pthread_detach(tid)) != 0)
	posix_error(rc, "Pthread_detach error");
}

void Pthread_exit(void *retval) {
    pthread_exit(retval);
}

pthread_t Pthread_self(void) {
    return pthread_self();
}
 
void Pthread_once(pthread_once_t *once_control, void (*init_function)()) {
    pthread_once(once_control, init_function);
}

void Pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  int rc;
  if ((rc = pthread_mutex_init(mutex, attr)) != 0)
    posix_error(rc, "Pthread_mutex_init error");
}

void P_P(pthread_mutex_t *mutex) {
  Pthread_mutex_lock(mutex);
}
void Pthread_mutex_lock(pthread_mutex_t *mutex) {
  int rc;
  if ((rc = pthread_mutex_lock(mutex)) != 0)
    posix_error(rc, "Pthread_mutex_lock error");
}

void P_V(pthread_mutex_t *mutex) {
  Pthread_mutex_unlock(mutex);
}
void Pthread_mutex_unlock(pthread_mutex_t *mutex) {
  int rc;
  if ((rc = pthread_mutex_unlock(mutex)) != 0)
    posix_error(rc, "Pthread_mutex_unlock error");
}

void Pthread_mutex_destroy(pthread_mutex_t *mutex) {
  int rc;
  if ((rc = pthread_mutex_destroy(mutex)) != 0)
    posix_error(rc, "Pthread_mutex_destroy error");
}

void Pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  int rc;
  if ((rc = pthread_cond_init(cond, attr)) != 0)
    posix_error(rc, "Pthread_cond_init error");
}

void Pthread_cond_destroy(pthread_cond_t  *cond) {
  int rc;
  if ((rc = pthread_cond_destroy(cond)) != 0)
    posix_error(rc, "Pthread_cond_signal error");
}

void Pthread_cond_signal(pthread_cond_t  *cond) {
  int rc;
  if ((rc = pthread_cond_signal(cond)) != 0)
    posix_error(rc, "Pthread_cond_signal error");
}

void Pthread_cond_broadcast(pthread_cond_t  *cond) {
  int rc;
  if ((rc = pthread_cond_broadcast(cond)) != 0)
    posix_error(rc, "Pthread_cond_broadcast error");
}

void Pthread_cond_wait(pthread_cond_t  *cond, pthread_mutex_t *mutex) {
  int rc;
  if ((rc = pthread_cond_wait(cond, mutex)) != 0)
    posix_error(rc, "Pthread_cond_wait error");
}



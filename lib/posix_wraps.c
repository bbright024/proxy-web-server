#include <includes/csapp.h>
#include <includes/errors.h>

/*******************************
 * Wrappers for Posix semaphores
 *******************************/

void Sem_init(sem_t *sem, int pshared, unsigned int value) 
{
    if (sem_init(sem, pshared, value) < 0)
	thread_unix_error("Sem_init error");
}

void Sem_destroy(sem_t *sem)
{
  if ((sem_destroy(sem)) < 0)
    thread_unix_error("Sem_destroy error");
}

void P(sem_t *sem) 
{
    if (sem_wait(sem) < 0)
	thread_unix_error("P error");
}

void V(sem_t *sem) 
{
    if (sem_post(sem) < 0)
	thread_unix_error("V error");
}


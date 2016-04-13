#include "private/threads.h"
#include "private/errors.h"
#include <pthread.h>

#include <stdlib.h>

#ifndef NDEBUG
#include <stdio.h>
#include <errno.h>
#define ENSURE(expr)    \
    int ret = expr;     \
    if (ret != 0) {     \
      errno = ret;      \
      perror(#expr);    \
      abort();          \
    }
#else
#define ENSURE(expr)    \
    if (expr) abort();
#endif

int bp__mutex_init(bp__mutex_t *mutex)
{
    return pthread_mutex_init(mutex, NULL) == 0 ? BP_OK : BP_EMUTEX;
}

void bp__mutex_destroy(bp__mutex_t *mutex)
{
    ENSURE(pthread_mutex_destroy(mutex));
}

void bp__mutex_lock(bp__mutex_t *mutex)
{
    ENSURE(pthread_mutex_lock(mutex));
}

void bp__mutex_unlock(bp__mutex_t *mutex)
{
    ENSURE(pthread_mutex_unlock(mutex));
}

int bp__rwlock_init(bp__rwlock_t *rwlock)
{
    return pthread_rwlock_init(rwlock, NULL) == 0 ? BP_OK : BP_ERWLOCK;
}

void bp__rwlock_destroy(bp__rwlock_t *rwlock)
{
    ENSURE(pthread_rwlock_destroy(rwlock));
}

void bp__rwlock_rdlock(bp__rwlock_t *rwlock)
{
    ENSURE(pthread_rwlock_rdlock(rwlock));
}

void bp__rwlock_wrlock(bp__rwlock_t *rwlock)
{
    ENSURE(pthread_rwlock_wrlock(rwlock));
}

void bp__rwlock_unlock(bp__rwlock_t *rwlock)
{
    ENSURE(pthread_rwlock_unlock(rwlock));
}

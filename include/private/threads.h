#ifndef _PRIVATE_THREADS_H_
#define _PRIVATE_THREADS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef pthread_mutex_t bp__mutex_t;
typedef pthread_rwlock_t bp__rwlock_t;


int bp__mutex_init(bp__mutex_t *mutex);
void bp__mutex_destroy(bp__mutex_t *mutex);
void bp__mutex_lock(bp__mutex_t *mutex);
void bp__mutex_unlock(bp__mutex_t *mutex);

int bp__rwlock_init(bp__rwlock_t *rwlock);
void bp__rwlock_destroy(bp__rwlock_t *rwlock);
void bp__rwlock_rdlock(bp__rwlock_t *rwlock);
void bp__rwlock_wrlock(bp__rwlock_t *rwlock);
void bp__rwlock_unlock(bp__rwlock_t *rwlock);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _PRIVATE_THREADS_H_ */

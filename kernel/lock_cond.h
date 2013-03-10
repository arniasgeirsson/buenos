#ifndef BUENOS_KERNEL_LOCK_COND_H
#define BUENOS_KERNEL_LOCK_COND_H

#include "kernel/spinlock.h"

typedef int cond_t;

typedef struct {
  int lock;
  spinlock_t slock;
} lock_t;  

int lock_reset(lock_t *lock);
void lock_acquire(lock_t *lock);
void lock_release(lock_t *lock);

void condition_init(cond_t *cond);
void condition_wait(cond_t *cond, lock_t *lock);
void condition_signal(cond_t *cond, lock_t *lock);
void condition_broadcast(cond_t *cond, lock_t *lock);

#endif /* BUENOS_KERNEL_LOCK_COND_H */

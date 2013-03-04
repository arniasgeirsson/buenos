#include "kernel/sleepq.h"
#include "kernel/tread.h"

int lock_reset(lock_t *lock){
  if(*lock == 0)
    return -1;
  
  *lock = 1;
  return 0;
}

void lock_acquire(lock_t *lock){
  if(*lock)
    *lock = 0;
  else{
    sleepq_add((void*)lock);
    thread_switch();
    lock_acquire(lock);
  }
}

void lock_release(lock_t *lock){
  *lock = 1;
  sleepq_wake((void*)lock);
}

void condition_init(cond_t *cond){
  cond = cond;
}

void condition_wait(cond_t *cond, lock_t *lock){
  sleepq_add((void*)cond);
  thread_switch();
}

void condition_signal(cond_t *cond, lock_t *lock){
  sleepq_wake((void*)cond);
}

void condition_broadcast(cond_t *cond, lock_t *lock){
  sleepq_wake_all((void*)cond);
}

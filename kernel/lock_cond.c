#include "kernel/lock_cond.h"
#include "kernel/interrupt.h"
#include "kernel/sleepq.h"
#include "kernel/thread.h"
#include "kernel/assert.h"

interrupt_status_t intr_status;

int lock_reset(lock_t *lock){
  intr_status = _interrupt_disable();
  //if(*lock == 0)
  //  return -1;
  
  spinlock_reset(&(lock->slock));
  
  spinlock_acquire(&(lock->slock));

  lock->lock = 1;

  spinlock_release(&(lock->slock));
  _interrupt_set_state(intr_status);
  return 0;
}

void lock_acquire(lock_t *lock){
  intr_status = _interrupt_disable();
  spinlock_acquire(&(lock->slock));

  while(!(lock->lock)){
    sleepq_add((void*)lock);
    thread_switch();
  }
  lock->lock = 0;
  
  spinlock_release(&(lock->slock));
  _interrupt_set_state(intr_status);
}

void lock_release(lock_t *lock){
  intr_status = _interrupt_disable();
  spinlock_acquire(&(lock->slock));
  
  lock->lock = 1;
  sleepq_wake((void*)lock);

  spinlock_release(&(lock->slock));
  _interrupt_set_state(intr_status);
}

void condition_init(cond_t *cond){
  intr_status = _interrupt_disable();
  *cond = 0;
  _interrupt_set_state(intr_status);
}

void condition_wait(cond_t *cond, lock_t *lock){
  intr_status = _interrupt_disable();
  KERNEL_ASSERT(cond == NULL);
  lock_release(lock);
  sleepq_add((void*)cond);
  thread_switch();
  lock_acquire(lock);
  _interrupt_set_state(intr_status);

}

void condition_signal(cond_t *cond, lock_t *lock){
  intr_status = _interrupt_disable();
  lock = lock;
  sleepq_wake((void*)cond);
  _interrupt_set_state(intr_status);
}

void condition_broadcast(cond_t *cond, lock_t *lock){
  intr_status = _interrupt_disable();
  lock = lock;
  sleepq_wake_all((void*)cond);
  _interrupt_set_state(intr_status);
}

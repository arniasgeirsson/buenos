#include "kernel/interrupt.h"
#include "kernel/sleepq.h"
#include "kernel/tread.h"

interrupt_status_t intr_status;

int lock_reset(lock_t *lock){
  intr_status = _interrupt_disable();
  if(*lock == 0)
    return -1;
  
  *lock = 1;
  return 0;
}

void lock_acquire(lock_t *lock){
  intr_status = _interrupt_disable();
  if(*lock)
    *lock = 0;
  else{
    sleepq_add((void*)lock);
    thread_switch();
    lock_acquire(lock);
  }
  _interrupt_set_state(intr_status);
}

void lock_release(lock_t *lock){
  intr_status = _interrupt_disable();
  *lock = 1;
  sleepq_wake((void*)lock);
  _interrupt_set_state(intr_status);
}

void condition_init(cond_t *cond){
  intr_status = _interrupt_disable();
  *cond = 1;
  _interrupt_set_state(intr_status);
}

void condition_wait(cond_t *cond, lock_t *lock){
  intr_status = _interrupt_disable();
  KERNEL_ASSERT(!(*cond));
  lock_release(lock);
  sleepq_add((void*)cond);
  thread_switch();
  lock_acquire(lock);
  _interrupt_set_state(intr_status);

}

void condition_signal(cond_t *cond, lock_t *lock){
  intr_status = _interrupt_disable();
  KERNEL_ASSERT(!(*lock));
  sleepq_wake((void*)cond);
  _interrupt_set_state(intr_status);
}

void condition_broadcast(cond_t *cond, lock_t *lock){
  intr_status = _interrupt_disable();
  sleepq_wake_all((void*)cond);
  _interrupt_set_state(intr_status);
}

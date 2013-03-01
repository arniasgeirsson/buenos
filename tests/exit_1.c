#include "tests/lib.h"

static const char prog1[] = "[disk1]prog1";
static const char prog3[] = "[disk1]prog3";

int main(void)
{
  wrapper_writeString("Starting syscall_exit tests!\n");
  int retval;

  /* 1. See if the correct value is passed on from the child process. */
  retval = syscall_join(syscall_exec(prog1));
  wrapper_writeMlt("1. Child exited with expected value: ",retval==0,"\n");

  /* 2. See if */
  retval = syscall_join(syscall_exec(prog3));
  wrapper_writeMlt("2. Child exited with expected value: ",retval==-2,"\n");

  wrapper_writeString("Finished syscall_exit tests!\n");

  syscall_exit(0); 

  return 0; 
}

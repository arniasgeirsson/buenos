#include "tests/lib.h"

static const char prog1[] = "[disk1]prog1";
static const char prog2[] = "[disk1]prog2";

int main(void)
{
  wrapper_writeString("Starting to test syscall_join!\n");

  int retval;
  int a;
  int b;

  /* ------ Part 1 ------ */
  /* 1. Join yourself. */
  retval = syscall_join(0);
  wrapper_writeMlt("1. Joined myself: ",retval == -3,"\n");
  
  /* 2. Join with index out of bounds. */
  retval = syscall_join(-42);
  wrapper_writeMlt("2. Joined out of bounds: ", retval == -1,"\n");

  /* 3. Join a process that does not exist. */
  retval = syscall_join(5);
  wrapper_writeMlt("3. Joined 'empty' process: ", retval==-2,"\n");

  /* ------ Part 2 ------ */
  /* 4. Join your own child. */
  retval = syscall_join(syscall_exec(prog1));
  wrapper_writeMlt("4. Joined my own child: ",retval==0,"\n");

  /* 5. Join your own zombie child. */
  a = syscall_exec(prog1);
  b = syscall_exec(prog1);
  syscall_join(b);
  /* By now I assume that a is a zombie. */
  retval = syscall_join(a);
  wrapper_writeMlt("5. Joined my own zombie child: ",retval==0,"\n");

  /* 6. Join a process that is not your own child. */
  /* Prog2 tries to join its parent, and returns the
     return value from its join call. */
  retval = syscall_join(syscall_exec(prog2));
  wrapper_writeMlt("6. Joined not your own child: ",retval==-4,"\n");

  wrapper_writeString("Finished testing syscall_join.\n");
  syscall_halt();
  return 0;
}

#include "tests/lib.h"

static const char validprog[] = "[disk1]validprog";
static const char prog1[] = "[disk1]prog1";

int main(void)
{
  wrapper_writeString("Starting the syscall_exec test!\n");

  int child;
  int ret;
  int max_processes = 32;
  int i;
  int children[max_processes];

  /* 1. Test the exec syscall by trying to execute a new process and join it. */
  wrapper_writeString("1: Trying to execute: ");
  wrapper_writeString((char*)validprog);
  wrapper_writeString("\n");

  child = syscall_exec(validprog);
  
  /* Check if any error happened. */
  if (child < 0) {
    wrapper_writeMlt("Error: Created child with error: ",child,"!\n");
  }
  wrapper_writeMlt("1.2: Created child: ",child,", trying to join it.\n");

  /* Try and join the child. */
  ret = syscall_join(child);

  wrapper_writeMlt("1.3: Return value from joined process was: ",ret,"\n");

  /* 2. Try and create more processes than allowed. */
  for (i=0; i < max_processes; i++) {
    children[i] = syscall_exec(prog1);
    /* Check if any error happened. */
    if (children[i] < 0) {
      wrapper_writeMlt("2. Error: Child number: ",i,", ");
      wrapper_writeMlt("was created with error: ",children[i],"\n");
    }
  }

  /* Wait for all the created processes to be finished before quiting. */
  for (i=0; i < max_processes; i++) {
    syscall_join(children[i]);
  }

  wrapper_writeString("Finished the syscall_exec test!\n");
  syscall_halt();
  return 0;
}

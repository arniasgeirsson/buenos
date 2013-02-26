#include "tests/lib.h"

static const char validprog[] = "[disk1]validprog";
static const char invalidprog[] = "[disk1]invalidprog";
static const char prog1[] = "[disk1]prog1";

int main(void)
{
  int child;
  int ret;
  int max_processes = 32;
  int i;
  int children[max_processes];

  /* Test the exec syscall by trying to execute a new process and join it. */
  wrapper_writeString("1-exec_1: Trying to execute: ");
  wrapper_writeString((char*)validprog);
  wrapper_writeString("\n");

  child = syscall_exec(validprog);
  
  /* Check if any error happened. */
  if (child < 0) {
    wrapper_writeMlt("Error: Created child with error: ",child,"!\n");
  }
  wrapper_writeMlt("2-exec_1: Created child: ",child,", trying to join it.\n");

  /* Try and join the child. */
  ret = syscall_join(child);

  wrapper_writeMlt("3-exec_1: Return value from joined process was: ",ret,"\n");

  /* Try and create more processes than allowed. */
  for (i=0; i < max_processes; i++) {
    children[i] = syscall_exec(prog1);
    /* Check if any error happened. */
    if (children[i] < 0) {
      wrapper_writeMlt("Error: Created child with error: ",children[i],"!\n");
    }
  }

  /* Wait for all the created processes to be finished before quiting. */
  for (i=0; i < max_processes; i++) {
    syscall_join(children[i]);
  }

  syscall_halt();
  return 0;
}

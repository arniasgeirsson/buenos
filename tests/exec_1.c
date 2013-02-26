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
  if (child < 0) {
    wrapper_writeMlt("Error: Exec_1, created child with error: ",child,"! Halting.\n");
    syscall_halt();
  }
  wrapper_writeMlt("2-exec_1: Creating child: ",child,", trying to join it.\n");

  ret = syscall_join(child);

  wrapper_writeMlt("3-exec_1: Return value from joined process was: ",ret,"\n");

  /* child = syscall_exec(invalidprog);*/ /* Is first caught in process_start. */
  for (i=0; i < max_processes; i++) {
    children[i] = syscall_exec(prog1);
    if (children[i] < 0) {
      wrapper_writeMlt("Error: Created child with error: ",children[i],"!\n");
    }
  }

  for (i=0; i < max_processes; i++) {
    syscall_join(children[i]);
  }

  syscall_halt();
  return 0;
}

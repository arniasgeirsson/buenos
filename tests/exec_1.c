#include "tests/lib.h"

static const char validprog[] = "[disk1]validprog";
static const char invalidprog[] = "[disk1]invalidprog";

int main(void)
{
  int child;
  int ret;

  wrapper_writeString("1-exec_1: Trying to execute: ");
  wrapper_writeString((char*)validprog);
  wrapper_writeString("\n");
  child = syscall_exec(validprog);
  wrapper_writeMlt("2-exec_1: Creating child: ",child,", trying to join it.\n");
  ret = syscall_join(child);
  wrapper_writeMlt("3-exec_1: Return value from joined process was: ",ret,"\n");

  /* child = syscall_exec(invalidprog);*/ /* Is first caught in process_start. */

  syscall_halt();
  return 0;
}

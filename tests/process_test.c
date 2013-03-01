#include "tests/lib.h"

static const char exec[] = "[disk1]exec_1";
static const char join[] = "[disk1]join_1";
static const char exit[] = "[disk1]exit_1";

int main(void)
{
  syscall_join(syscall_exec(exec));
  syscall_join(syscall_exec(join));
  syscall_join(syscall_exec(exit));

  syscall_halt();
  return 0;
}

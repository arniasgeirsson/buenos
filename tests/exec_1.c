#include "tests/lib.h"

static const char validprog[] = "[disk1]validprog";
static const char invalidprog[] = "[disk1]invalidprog";

int main(void)
{
  int child;
  int ret;

  syscall_write(1,"Onnne\n",6);
  child = syscall_exec(validprog);
  syscall_write(1,"Twooo\n",6);
  ret = syscall_join(child);
  syscall_write(1,"Three\n",6);

  child = syscall_exec(invalidprog);

  child = child;
  ret = ret;
  syscall_halt();
  return 0;
}

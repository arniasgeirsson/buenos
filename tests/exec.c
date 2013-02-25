/*
 * Userland exec test
 */

#include "tests/lib.h"

static const char prog[] = "[disk1]hw"; /* The program to start. */

int main(void)
{
  int child;
  int ret;
  char str[20];
  int a = 1;
  /* syscall_write("Starting program %s\n", prog);*/
  syscall_write(1,"One\n",4);
  child = syscall_exec(prog);
  str[0] = (char)child;
  str[1] = (char)a;
  str[2] = '\0';
  syscall_write(1,str,20);
  /* syscall_write("Now joining child %d\n", child);*/
  syscall_write(1,"Two\n",4);
  child = child;
  ret = (char)syscall_join(child);
  /*syscall_write("Child joined with status: %d\n", ret);*/
  syscall_write(1,"Three\n",6);
  ret = ret;
  syscall_halt();
  return 0;
}
